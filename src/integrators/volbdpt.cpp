
/*
    pbrt source code Copyright(c) 1998-2012 Matt Pharr and Greg Humphreys.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */


// integrators/volpath.cpp*
#include "stdafx.h"
#include "volbdpt.h"
#include "integrators/volbdpt.h"
#include "volpath.h"
#include "scene.h"
#include "paramset.h"
#include "montecarlo.h"

namespace pbrt
{

// VolumeBDPTIntegrator Method Definitions
void VolumeBDPTIntegrator::RequestSamples(Sampler *sampler, Sample *sample,
        const Scene *scene) {
    tauSampleOffset = sample->Add1D(1);
    scatterSampleOffset = sample->Add1D(1);
}


Spectrum VolumeBDPTIntegrator::Transmittance(const Scene *scene,
        const Renderer *renderer, const RayDifferential &ray,
        const Sample *sample, RNG &rng, MemoryArena &arena) const {
    if (!scene->volumeRegion) return Spectrum(1.f);
    float step, offset;
    if (sample) {
        step = stepSize;
        offset = sample->oneD[tauSampleOffset][0];
    }
    else {
        step = 4.f * stepSize;
        offset = rng.RandomFloat();
    }
    Spectrum tau = scene->volumeRegion->tau(ray, step, offset);
    return Exp(-tau);
}


Spectrum VolumeBDPTIntegrator::EstimateDirectLight(const Scene *scene,
        const Renderer *renderer, MemoryArena &arena, const Light *light,
        const Point &p, const Normal &n, const Vector &wo, float rayEpsilon,
        float time, RNG &rng, const LightSample &lightSample) const {
    VolumeRegion *vr = scene->volumeRegion;
    if (!vr) Spectrum(0.);
    Spectrum Ld(0.);
    // Sample light source.
    Vector wi;
    float lightPdf;
    VisibilityTester visibility;
    Spectrum Li = light->Sample_L(p, rayEpsilon, lightSample, time,
                                  &wi, &lightPdf, &visibility);
    if (lightPdf > 0. && !Li.IsBlack() && visibility.Unoccluded(scene)) {
        // Add light's contribution to reflected radiance
        Li *= visibility.Transmittance(scene, renderer, NULL, rng, arena);
        Ld += vr->p(p, -wi, wo, time) * vr->Sigma_s(p, wo, time) * Li / lightPdf;
    }
    return Ld;
}


Spectrum VolumeBDPTIntegrator::UniformSampleLight(const Scene *scene,
        const Renderer *renderer, MemoryArena &arena, const Point &p,
        const Normal &n, const Vector &wo, float rayEpsilon, float time, RNG &rng) const {
    // Randomly choose a single light to sample, _light_
    int nLights = int(scene->lights.size());
    if (nLights == 0) return Spectrum(0.);
    int lightNum;
    lightNum = Floor2Int(rng.RandomFloat() * nLights);
    lightNum = min(lightNum, nLights-1);
    Light *light = scene->lights[lightNum];

    // Initialize light sample for single light sampling
    LightSample lightSample(rng);

    return (float) nLights * EstimateDirectLight(scene, renderer, arena,
            light, p, n, wo, rayEpsilon, time, rng, lightSample);
}


void VolumeBDPTIntegrator::Preprocess(const Scene* scene, const Camera* camera,
                                      const Renderer* renderer) {
    // Do any pre-processing before rendeirng
}


void VolumeBDPTIntegrator::EyeRandomWalk(const Scene *scene, const Ray &eyeRay,
        VolumeVertexList& vertexList, RNG &rng) const {
    // Do a random walk for the eye ray in the volume
    Spectrum cummulative(1.f);

    // Find the intersection between the eye ray and the volume
    VolumeRegion *vr = scene->volumeRegion;
    float t0, t1;
    if (!vr || !vr->IntersectP(eyeRay, &t0, &t1) || (t1-t0) == 0.f || t0 < 0.f) {
        return;
    }

    // Find the intersection point between the sampled light ray and the volume
    RayDifferential ray(eyeRay);
    Point p = ray(t0), pPrev;
    uint64_t bounces = 0;
    while(vr->WorldBound().Inside(p)) {
        Vector wi = -ray.d;
        const Spectrum sigma_a = vr->Sigma_a(p, wi, eyeRay.time);
        const Spectrum sigma_s = vr->Sigma_s(p, wi, eyeRay.time);
        const Spectrum STER = vr->STER(p, wi, eyeRay.time);
        // Construct and add the _eyeVertex_ to the _vertexList_
        VolumeVertex eyeVertex(p, wi, sigma_a, sigma_s, cummulative, 1.0);
        vertexList.push_back(eyeVertex);

        // Sample the direction of the next event
        float directionPdf = 1.f;
        Vector wo;
        if(STER.y() > rng.RandomFloat()) {
            // Change the ray direction due to a scattering event at _p_
            if(!vr->SampleDirection(p, wi, wo, &directionPdf, rng)) {
                break; // Direction error
            }

            // Account for the losses due to the scattering event at _p_
            cummulative *= sigma_s * vr->p(p, wi, wo, ray.time);
        } else {
            // Account only for the trnsmittance between the previous and the
            // next events becuse there is no direction change.
            wo = ray.d;
        }

        // Sample the distance of the next event
        ray = RayDifferential(p, wo, 0, INFINITY);

        float tDist;
        float distancePdf = 1.f;
        Point Psample;
        if(!vr->SampleDistance(ray, &tDist, Psample, &distancePdf, rng)) {
            break; // The sampled point is outside the volume
        }

        // Account for the sampling Pdfs from sampling a direction and/or distance
        const float pdf = distancePdf * directionPdf;
        cummulative *= 1 / pdf;
        // cummulative *= PowerHeuristic(1, distancePdf, 1, directionPdf);

        // Update the events and account for the transmittance between the events
        pPrev = p;
        p = Psample;
        const Ray tauRay(pPrev, p - pPrev, 0.f, 1.f, ray.time, ray.depth);
        const Spectrum stepTau = vr->tau(tauRay, .5f * stepSize, rng.RandomFloat());
        const Spectrum TrPP = Exp(-stepTau);
        cummulative *= TrPP;

        // Possibly terminate ray marching if _cummulative_ is small
        if (cummulative.y() < 1e-3) {
            const float continueProb = .5f;
            if (rng.RandomFloat() > continueProb) {
                cummulative = 0.f;
                break;
            }
            cummulative /= continueProb;
        }

        // Terminate if bounces are more than requested
        bounces++;
        if (bounces > maxEyeDepth) {
            break;
        }
    }
}


void VolumeBDPTIntegrator::LightRandomWalk(const Scene *scene,
        VolumeVertexList& vertexList, RNG &rng) const {
    // Sample a light source and bounce a ray into the scene
    // Choose light source to trace virtual light path from
    float lightSourcesPdf;
    Distribution1D *lightDistribution = ComputeLightSamplingCDF(scene);
    int ln = lightDistribution->SampleDiscrete(rng.RandomFloat(), &lightSourcesPdf);
    Light *light = scene->lights[ln];

    // Sample a ray leaving the light source for constructing a light path
    RayDifferential ray;
    float lightPdf;
    LightSample ls(rng.RandomFloat(), rng.RandomFloat(), rng.RandomFloat());
    Normal Nl;
    Spectrum Li = light->Sample_L(scene, ls, rng.RandomFloat(),
            rng.RandomFloat(), 0, &ray, &Nl, &lightPdf);
    if (lightPdf == 0.f || Li.IsBlack()) return;

    // Account for the light pdf
    Li *= 1 / (lightPdf * lightSourcesPdf);

    // Find the intersection between the ray and the volume
    VolumeRegion *vr = scene->volumeRegion;
    float t0, t1;
    if (!vr || !vr->IntersectP(ray, &t0, &t1) || (t1-t0) == 0.f || t0 < 0.f) {
        return;
    }

    // Importance that corresponds to the actual light radiance
    Spectrum cummulative = Li;

    // Find the intersection point between the sampled light ray and the volume
    Point p = ray(t0), pPrev;
    uint64_t bounces = 0;
    while(vr->WorldBound().Inside(p)) {
        // Construct and add the _eyeVertex_ to the _vertexList_
        Vector wi = -ray.d;
        const Spectrum simga_a = vr->Sigma_a(p, wi, ray.time);
        const Spectrum sigma_s = vr->Sigma_s(p, wi, ray.time);
        const Spectrum STER = vr->STER(p, wi, ray.time);
        VolumeVertex vertex(p, wi, simga_a, sigma_s, cummulative, 1.0);
        vertexList.push_back(vertex);

        // Sample the direction of the next event
        float directionPdf = 1.f;
        Vector wo;
        if (STER.y() > rng.RandomFloat()) {
            // Change ray direction due to a scattering event at _p_.
            if(!vr->SampleDirection(p, wi, wo, &directionPdf, rng)) {
                break;
            }

            // Account for the losses due to the scattering event at _p_
            cummulative *= sigma_s * vr->p(p, wi, wo, ray.time);
        } else {
            // Account only for the trnsmittance between the previous and the
            // next events becuse there is no direction change.
            wo = ray.d;
        }

        // Construct the new ray to sample the next interaction point
        ray = RayDifferential(p, wo, ray, 0, INFINITY);

        // Sample a new distance in the medium
        float distancePdf;
        float tDist;
        Point Psample;
        if(!vr->SampleDistance(ray, &tDist, Psample, &distancePdf, rng)) {
            break;
        }

        // Account for the sampling Pdfs from sampling a direction and/or distance
        const float pdf = distancePdf * directionPdf;
        cummulative *= 1 / pdf;
        // cummulative *= PowerHeuristic(1, distancePdf, 1, directionPdf);

        pPrev = p;
        p = Psample;

        // Compute the transmittance between _pPrev_ & _p_
        Ray tauRay(pPrev, p - pPrev, 0.f, 1.f, ray.time, ray.depth);
        Spectrum stepTau = vr->tau(tauRay, .5f * stepSize, rng.RandomFloat());
        cummulative *= Exp(-stepTau);

        // Possibly terminate ray marching if transmittance is small
        if (cummulative.y() < 1e-3) {
            const float continueProb = .5f;
            if (rng.RandomFloat() > continueProb) {
                cummulative = 0.f;
                break;
            }
            cummulative /= continueProb;
        }
        bounces++;
        if (bounces > maxLightDepth) {
            break;
        }
    }
}


Spectrum VolumeBDPTIntegrator::Li_Single(const Scene *scene, const Renderer *renderer,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        Spectrum *T, MemoryArena &arena) const {
    VolumeRegion *vr = scene->volumeRegion;
    float t0, t1;
    if (!vr || !vr->IntersectP(ray, &t0, &t1) || (t1-t0) == 0.f) {
        *T = 1.f;
        return 0.f;
    }
    // Do single scattering volume integration in _vr_
    Spectrum Lv(0.);

    // Prepare for volume integration stepping
    int nSamples = Ceil2Int((t1-t0) / stepSize);
    float step = (t1 - t0) / nSamples;
    Spectrum Tr(1.f);
    Point p = ray(t0), pPrev;
    Vector w = -ray.d;
    t0 += sample->oneD[scatterSampleOffset][0] * step;

    // Compute sample patterns for single scattering samples
    float *lightNum = arena.Alloc<float>(nSamples);
    LDShuffleScrambled1D(1, nSamples, lightNum, rng);
    float *lightComp = arena.Alloc<float>(nSamples);
    LDShuffleScrambled1D(1, nSamples, lightComp, rng);
    float *lightPos = arena.Alloc<float>(2*nSamples);
    LDShuffleScrambled2D(1, nSamples, lightPos, rng);
    uint32_t sampOffset = 0;
    for (int i = 0; i < nSamples; ++i, t0 += step) {
        // Advance to sample at _t0_ and update _T_
        pPrev = p;
        p = ray(t0);
        Ray tauRay(pPrev, p - pPrev, 0.f, 1.f, ray.time, ray.depth);
        Spectrum stepTau = vr->tau(tauRay,
                                   .5f * stepSize, rng.RandomFloat());
        Tr *= Exp(-stepTau);

        // Possibly terminate ray marching if transmittance is small
        if (Tr.y() < 1e-3) {
            const float continueProb = .5f;
            if (rng.RandomFloat() > continueProb) {
                Tr = 0.f;
                break;
            }
            Tr /= continueProb;
        }

        // Compute single-scattering source term at _p_
        Lv += Tr * vr->Lve(p, w, ray.time);
        Spectrum ss = vr->Sigma_s(p, w, ray.time);
        if (!ss.IsBlack() && scene->lights.size() > 0) {
            int nLights = scene->lights.size();
            int ln = min(Floor2Int(lightNum[sampOffset] * nLights),
                         nLights-1);
            Light *light = scene->lights[ln];
            // Add contribution of _light_ due to scattering at _p_
            float pdf;
            VisibilityTester vis;
            Vector wo;
            LightSample ls(lightComp[sampOffset], lightPos[2*sampOffset],
                           lightPos[2*sampOffset+1]);
            Spectrum L = light->Sample_L(p, 0.f, ls, ray.time, &wo, &pdf, &vis);
            if (!L.IsBlack() && pdf > 0.f && vis.Unoccluded(scene)) {
                Spectrum Ld = L * vis.Transmittance(scene, renderer, NULL, rng, arena);
                Lv += Tr * ss * vr->p(p, w, -wo, ray.time) * Ld * float(nLights) /
                        pdf;
            }
        }
        ++sampOffset;
    }
    *T = Tr;
    return Lv * step;
}


Spectrum VolumeBDPTIntegrator::EvaluatePath(const Scene *scene,
        const VolumeVertexList &eyePath, const VolumeVertexList &lightPath,
        const uint64_t nEye, const uint64_t nLight, RNG &rng) const {
    // Evaluate the path contribution for a specific path
    const VolumeVertex &ev = eyePath[nEye];
    const VolumeVertex &lv = lightPath[nLight];

    // Start with an importance of Spectrum(1.0)
    Spectrum L(1.0);

    // Account for the input importance contribution at the two vertecies
    L *= ev.pathContrib * lv.pathContrib;

    // Calculate and account for the geometric term between the two points
    Vector etl = lv.p - ev.p;
    const float lengthSquared = etl.LengthSquared();
    // Extremely close points cause numerical problems
    if(lengthSquared < 0.05)
        return Spectrum(0.f);

    etl /= sqrt(lengthSquared);
    const float geomTerm = 1 / lengthSquared;
    L *= geomTerm;

    // Calculate and account for the scattering at the two vertecies
    // Directions are computed according to the incident directions
    const VolumeRegion *vr = scene->volumeRegion;
    const Vector eWi = ev.wi; const Vector eWo = etl;
    const Vector lWi = lv.wi; const Vector lWo = -etl;
    L *= vr->Sigma_s(ev.p, eWi, 0 /* ray.time */) * vr->p(ev.p, eWi, eWo, 0 /* ray.time */);
    L *= vr->Sigma_s(lv.p, lWo, 0 /* ray.time */) * vr->p(lv.p, lWi, lWo, 0 /* ray.time */);

    // Calculate and account for the transmittance between the two vertecies
    Ray tauRay(ev.p, lv.p - ev.p, 0.f, 1.f, 0 /* ray.time */, 0 /* ray.depth */);
    Spectrum stepTau = vr->tau(tauRay, .5f * stepSize, rng.RandomFloat());
    L *= Exp(-stepTau);

    return L;
}


///
/// \brief VolumePathIntegrator::Li_Multiple
/// \note In this function, we will use the same convention that is being used
/// in the Li_Single() function.
/// We will indicate by w to the direction where the light is propagated from
/// the point and towards the eye.
/// We will indicate by wo the direction of the ray from the light source after
/// being sampled to the point in the volume.
/// \param scene
/// \param renderer
/// \param r
/// \param sample
/// \param rng
/// \param T
/// \param arena
/// \return
///
Spectrum VolumeBDPTIntegrator::Li_Multiple(const Scene *scene, const Renderer *renderer,
        const RayDifferential &r, const Sample *sample, RNG &rng,
        Spectrum *T, MemoryArena &arena) const {
    RayDifferential ray(r);

    // Do eye and light random walks
    VolumeVertexList eyeVertexList;
    VolumeVertexList lightVertexList;
    EyeRandomWalk(scene, ray, eyeVertexList, rng);
    LightRandomWalk(scene, lightVertexList, rng);

    // Do multiple scattering
    Spectrum Li(0.f);
    for (uint64_t i = 0; i < eyeVertexList.size(); i++) {
        for (uint64_t j = 0; j < lightVertexList.size(); j++) {
            Li += EvaluatePath(scene, eyeVertexList, lightVertexList, i, j, rng);
        }
    }

    return Li;
}



Spectrum VolumeBDPTIntegrator::Li(const Scene *scene, const Renderer *renderer,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        Spectrum *T, MemoryArena &arena) const {
    // If no volumes, return null contribution and full _T_
    VolumeRegion *vr = scene->volumeRegion;
    float t0, t1;
    if (!vr || !vr->IntersectP(ray, &t0, &t1) || (t1-t0) == 0.f) {
        *T = 1.f;
        return Spectrum(0.f);
    }
    // Skip the empty space
//    float tSkip;
//    if (!vr->SkipEmptySpace(ray, &tSkip)) return Spectrum(0.);

    // If there are no light sources in the scene, then return Spectrum(0.)
    if (scene->lights.size() == 0) {
        return Spectrum(0.f);
    }

    // TODO: Should I average the results here from the two contributions !
    const Spectrum L_Single = Li_Single(scene, renderer, ray, sample, rng, T, arena);
    const Spectrum L_Multiple = Li_Multiple(scene, renderer, ray, sample, rng, T, arena);
    return (L_Single + L_Multiple);
}


VolumeBDPTIntegrator *CreateVolumeBDPTIntegrator(const ParamSet &params) {
    float stepSize  = params.FindOneFloat("stepsize", 1.f);
    uint64_t maxEyeDepth = params.FindOneInt("eyedepth", 3);
    uint64_t maxlightDepth = params.FindOneInt("lightdepth", 3);
    return new VolumeBDPTIntegrator(stepSize, maxEyeDepth, maxlightDepth);

}

}
