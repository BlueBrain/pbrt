
/*
    pbrt source code Copyright(c) 1998-2012 Matt Pharr and Greg Humphreys.
                                  2012-2016 Marwan Abdellah.

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


// integrators/fluorescencesingle.cpp*
#include "stdafx.h"
#include "integrators/fluorescencesingle.h"
#include "scene.h"
#include "paramset.h"
#include "montecarlo.h"
#include "stdio.h"

namespace pbrt
{

// SingleScatteringFluorescenceIntegrator Method Definitions
void SingleScatteringFluorescenceIntegrator::RequestSamples(Sampler *sampler,
        Sample *sample, const Scene *scene) {
    tauSampleOffset = sample->Add1D(1);
    scatterSampleOffset = sample->Add1D(1);
}


Spectrum SingleScatteringFluorescenceIntegrator::Transmittance(const Scene *scene,
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


void SingleScatteringFluorescenceIntegrator::Preprocess(const Scene *scene,
        const Camera *camera, const Renderer *renderer) {
    // Verify the volume contents of the scene
    VolumeRegion *vr = scene->volumeRegion;
    if(vr->HasNonClearedFluorescentVolumes()) {
        Error(" The SingleScatteringFluorescenceIntegrator only works with"
              " cleared fluorescent volumes");
    }

    // Verify laser light sources
    for (int i = 0; i < scene->lights.size(); i++) {
        Light* light = scene->lights[i];
        if (!light->IsLaser()) {
            Error("The fluorescence integrators can only use laser beams");
        }
    }
}


Spectrum SingleScatteringFluorescenceIntegrator::Li(const Scene *scene,
        const Renderer *renderer, const RayDifferential &ray,
        const Sample *sample, RNG &rng, Spectrum *T, MemoryArena &arena) const {
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
        Spectrum stepTau = vr->tau(tauRay, 0.5f * stepSize, rng.RandomFloat());
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

        // Compute fluorescence emission
        Spectrum sigma = vr->Mu(p, w, ray.time);
        if (!sigma.IsBlack() && scene->lights.size() > 0) {
            int nLights = scene->lights.size();
            int ln = min(Floor2Int(lightNum[sampOffset] * nLights),
                         nLights-1);
            Light *light = scene->lights[ln];

            // Add contribution of _light_ due to the in-scattering at _p_
            float pdf;
            VisibilityTester vis;
            Vector wo;
            LightSample ls(lightComp[sampOffset], lightPos[2*sampOffset],
                           lightPos[2*sampOffset+1]);
            Spectrum L = light->Sample_L(p, 0.f, ls, ray.time, &wo, &pdf, &vis);
            if (!L.IsBlack() && pdf > 0.f && vis.Unoccluded(scene)) {
                Spectrum Ld = L * vis.Transmittance(scene, renderer, NULL, rng,
                                                    arena);
                int lambdaExcIndex = light->GetLaserWavelengthIndex();
                float Lpower = Ld.GetLaserEmissionPower(lambdaExcIndex);
                float yield = vr->Yeild(p);
                Spectrum fEx = vr->fEx(p);
                Spectrum fEm = vr->fEm(p);
                float scale = fEx.GetSampleValueAtWavelengthIndex(lambdaExcIndex);
                Lv += Lpower * Tr * sigma * vr->pf(p, w, -wo, ray.time) *
                        scale * fEm * yield * float(nLights) / pdf;
#ifdef DEBUG
                printf("%f %f %f %f %f %f %f \n",
                       Lpower, Tr.y(), sigma.y(), vr->pf(p, w, -wo, ray.time),
                       scale, fEm.y(), yield);
#endif
            }
        }
        ++sampOffset;
    }
    *T = Tr;
    return Lv * step;
}


SingleScatteringFluorescenceIntegrator
*CreateSingleScatteringFluorescenceIntegrator(const ParamSet &params) {
    float stepSize  = params.FindOneFloat("stepsize", 1.f);
    return new SingleScatteringFluorescenceIntegrator(stepSize);
}

}
