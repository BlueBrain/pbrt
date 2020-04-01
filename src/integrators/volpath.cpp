
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
#include "integrators/volpath.h"
#include "volpath.h"
#include "scene.h"
#include "paramset.h"
#include "montecarlo.h"
#include "volume.h"

namespace pbrt
{

// VolumePathIntegrator Method Definitions
void VolumePatIntegrator::RequestSamples(Sampler *sampler, Sample *sample,
        const Scene *scene) {
    tauSampleOffset = sample->Add1D(1);
    scatterSampleOffset = sample->Add1D(1);
}


Spectrum VolumePatIntegrator::Transmittance(const Scene *scene,
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


Spectrum VolumePatIntegrator::EstimateDirectLight(const Scene *scene,
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
        // Li *= PowerHeuristic(1, lightPdf, 1, (1/M_PI_4));
        Ld += vr->p(p, -wi, wo, time) * vr->Sigma_s(p, wo, time) * Li / lightPdf;
    }
    return Ld;
}


Spectrum VolumePatIntegrator::UniformSampleLight(const Scene *scene,
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


void VolumePatIntegrator::Preprocess(const Scene* scene, const Camera* camera,
                                      const Renderer* renderer) {
    // Do any pre-processing before rendeirng
}


Spectrum VolumePatIntegrator::Li_Single(const Scene *scene, const Renderer *renderer,
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


void VolumePatIntegrator::EyeRandomWalk(const Scene *scene, const Ray &eyeRay,
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
        if (bounces > maxDepth) {
            break;
        }
    }
}


Spectrum VolumePatIntegrator::EvaluatePath(const Scene *scene,
        const VolumeVertexList &eyePath, const uint64_t nEye, RNG &rng,
        const Renderer *renderer, MemoryArena &arena) const {
    // Evaluate the path contribution for a specific path
    const VolumeVertex &ev = eyePath[nEye];

    // Start with an importance of Spectrum(1.0)
    Spectrum L(1.0);

    // Account for the input importance contribution at the two vertecies
    const Normal n;
    L *= ev.pathContrib * UniformSampleLight(scene, renderer, arena, ev.p, n, ev.wi, 0, 0, rng);
    return L;
}


Spectrum VolumePatIntegrator::Li_Multiple(const Scene *scene, const Renderer *renderer,
        const RayDifferential &r, const Sample *sample, RNG &rng,
        Spectrum *T, MemoryArena &arena) const {

    // Do eye and light random walks
    VolumeVertexList eyeVertexList;
    EyeRandomWalk(scene, r, eyeVertexList, rng);

    // Do multiple scattering
    Spectrum Li(0.f);
    for (uint64_t i = 0; i < eyeVertexList.size(); i++) {
        Li += EvaluatePath(scene, eyeVertexList, i, rng, renderer, arena);
    }

    return Li;
}

Spectrum VolumePatIntegrator::Li(const Scene *scene, const Renderer *renderer,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        Spectrum *T, MemoryArena &arena) const {
    // If no volumes, return null contribution and full _T_
    VolumeRegion *vr = scene->volumeRegion;
    float t0, t1;
    if (!vr || !vr->IntersectP(ray, &t0, &t1) || (t1-t0) == 0.f) {
        *T = 1.f;
        return Spectrum(0.f);
    }

    // If there are no light sources in the scene, then return Spectrum(0.)
    if (scene->lights.size() == 0) {
        return Spectrum(0.f);
    }

    return  Li_Single(scene, renderer, ray, sample, rng, T, arena) +
            Li_Multiple(scene, renderer, ray, sample, rng, T, arena);
}


VolumePatIntegrator *CreateVolumePathIntegrator(const ParamSet &params) {
    float stepSize  = params.FindOneFloat("stepsize", 1.f);
    uint64_t maxDepth = params.FindOneInt("depth", 3);
    return new VolumePatIntegrator(stepSize, maxDepth);
}

}
