
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


// integrators/fvpl.cpp*
#include "stdafx.h"
#include "fvpl.h"
#include "scene.h"
#include "paramset.h"
#include "montecarlo.h"
#include "camera.h"
#include <stdio.h>
#include <stdlib.h>

namespace pbrt
{

// FVPLIntegrator Method Definitions
void FVPLIntegrator::RequestSamples(Sampler *sampler, Sample *sample,
        const Scene *scene) {
    tauSampleOffset = sample->Add1D(1);
    scatterSampleOffset = sample->Add1D(1);
    vlSetOffset = sample->Add1D(1);
}


Spectrum FVPLIntegrator::Transmittance(const Scene *scene,
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


void FVPLIntegrator::WriteVPLs() {
    FILE* file = fopen("vpls", "w");
    for (uint32_t s = 0; s < nLightSets; ++s) {
        fprintf(file, "Set [%d] \n", s);
        for (uint32_t i = 0; i < nLightPaths; ++i) {
            const VPL &vl = vpls[s][i];
            fprintf(file, "\t[%d] %f %f %f \n", i, vl.p.x, vl.p.y, vl.p.z);
        }
    }
    fclose(file);
}


Spectrum FVPLIntegrator::Transmittance(const Scene *scene, const Point &p1,
        const Point &p2, RNG rng) const {
    Ray tauRay(p1, p2 - p1, 0.f, 1.f, 0, 0);
    Spectrum stepTau = scene->volumeRegion->tau(tauRay, .5f * stepSize,
                                                rng.RandomFloat());
    return Exp(-stepTau);
}


void FVPLIntegrator::Preprocess(const Scene *scene, const Camera *camera,
        const Renderer *renderer) {
    if (scene->lights.size() == 0) return;
    RNG rng;
    // Compute samples for emitted rays from lights
    vector<float> lightNum(nLightPaths * nLightSets);
    vector<float> lightSampPos(2 * nLightPaths * nLightSets, 0.f);
    vector<float> lightSampComp(nLightPaths * nLightSets, 0.f);
    vector<float> lightSampDir(2 * nLightPaths * nLightSets, 0.f);
    LDShuffleScrambled1D(nLightPaths, nLightSets, &lightNum[0], rng);
    LDShuffleScrambled2D(nLightPaths, nLightSets, &lightSampPos[0], rng);
    LDShuffleScrambled1D(nLightPaths, nLightSets, &lightSampComp[0], rng);
    LDShuffleScrambled2D(nLightPaths, nLightSets, &lightSampDir[0], rng);

    // Precompute information for light sampling densities
    Distribution1D *lightDistribution = ComputeLightSamplingCDF(scene);

    VolumeRegion *vr = scene->volumeRegion;
    if (!vr) return;

    Point p, pPrev;
    for (uint32_t s = 0; s < nLightSets; ++s) {
        for (uint32_t i = 0; i < nLightPaths; ++i) {
            // Follow path _i_ from light to create virtual lights
            int sampOffset = s*nLightPaths + i;

            // Choose light source to trace virtual light path from
            float lightPdf;
            int ln = lightDistribution->SampleDiscrete(lightNum[sampOffset],
                                                       &lightPdf);
            Light *light = scene->lights[ln];

            // Sample ray leaving light source for virtual light path
            RayDifferential ray;
            float pdf;
            LightSample ls(lightSampPos[2*sampOffset],
                           lightSampPos[2*sampOffset+1],
                           lightSampComp[sampOffset]);
            Normal Nl;
            Spectrum alpha = light->Sample_L(scene, ls,
                                             lightSampDir[2*sampOffset],
                                             lightSampDir[2*sampOffset+1],
                                             camera->shutterOpen,
                                             &ray, &Nl, &pdf);
            // If the light is not emitting, return
            if (pdf == 0.f || alpha.IsBlack()) continue;
            alpha *= AbsDot(Nl, ray.d) / (pdf * lightPdf);

            // If the ray doesn't intersect the volume, return
            float t0, t1;
            if (!vr->IntersectP(ray, &t0, &t1) || (t1-t0) == 0.f) continue;
            p = ray(t0);
            ray = RayDifferential(p, ray.d, 0, INFINITY, ray.time, ray.depth);
            while(vr->WorldBound().Inside(p)) {

                vpls[s].push_back(VPL(p, alpha, -ray.d));

                // Sample direction
                const Vector wi = -ray.d;
                Vector wo;
                float pdfDir;
                if(!vr->SampleDirection(p, wi, wo, &pdfDir, rng)) break;

                // Sample distance
                float tDist = 0;
                float pdfDist = 0;
                Point Psample;
                bool dist = vr->SampleDistance(ray, &tDist, Psample, &pdfDist, rng);
                // printf("tDist %f\n", tDist);
                if(!dist) break;

                pPrev = p;
                p = ray(tDist);
                // printf("%f %f %f \n", p.x, p.y, p.z);
                ray = RayDifferential(pPrev, wo, 0, tDist, ray.time, ray.depth + 1);

                // Transmittance, sigma scattering, and phase function
                alpha *= Transmittance(scene, pPrev, p, rng) / (pdfDir * pdfDist);
                alpha *= vr->Sigma_s(p, wo, ray.time) * vr->p(p, wi, wo, ray.time);
            }
        }
    }

    // Write the VPL to a file to check them
    WriteVPLs();
}


Spectrum FVPLIntegrator::Li(const Scene *scene, const Renderer *renderer,
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

        // Compute emission term at _p_
        Lv += Tr * vr->Lve(p, w, ray.time);

        // Compute multiple-scattering source term at _p_ due to the VPLs
        Spectrum ss = vr->Sigma_s(p, w, ray.time);
        if (!ss.IsBlack() && vpls.size() > 0) {

            // Compute indirect illumination with virtual lights
            uint32_t lSet = min(uint32_t(sample->oneD[vlSetOffset][0] * nLightSets),
                                nLightSets-1);
            for (uint32_t i = 0; i < vpls[lSet].size(); ++i) {
                const VPL &vl = vpls[lSet][i];
                Spectrum pathContrib = vl.pathContrib;

                // Transmittance, sigma scattering, and phase function at VPL and _p_
                Vector wi = Normalize(p - vl.p);
                pathContrib *= Transmittance(scene, vl.p, p, rng);
                pathContrib *= vr->Sigma_s(vl.p, wi, ray.time) * vr->p(vl.p, vl.w, wi, ray.time);
                pathContrib *= vr->Sigma_s(p, w, ray.time) * vr->p(p, -wi, w, ray.time);

                // Compute virtual light's tentative contribution _Llight_
                float d2 = DistanceSquared(p, vl.p);
                float G = 1 / d2;
                G = min(G, gLimit);
                pathContrib *= G;

                Lv += Tr * pathContrib;
            }
        }
    }
    *T = Tr;
    return Lv * step;
}


FVPLIntegrator *CreateFVPLIntegrator(const ParamSet &params) {
    int minLambda = params.FindOneInt("minLambda", sampledLambdaStart);
    int maxLambda = params.FindOneInt("maxLambda", sampledLambdaEnd);
    float stepSize  = params.FindOneFloat("stepsize", 1.f);
    int nLightPaths = params.FindOneInt("nlights", 4);
    if (PbrtOptions.quickRender) nLightPaths = max(1, nLightPaths / 4);
    int nLightSets = params.FindOneInt("nsets", 4);
    float glimit = params.FindOneFloat("glimit", 1000.f);
    return new FVPLIntegrator(stepSize, nLightPaths, nLightSets, glimit);
}

}
