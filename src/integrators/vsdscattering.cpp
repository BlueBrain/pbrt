
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


// integrators/vsdscattering.cpp*
#include "stdafx.h"
#include "vsdscattering.h"
#include "scene.h"
#include "paramset.h"
#include "montecarlo.h"

namespace pbrt
{

// VSDScatteringIntegrator Method Definitions
void VSDScatteringIntegrator::RequestSamples(Sampler *sampler, Sample *sample,
        const Scene *scene) {
    tauSampleOffset = sample->Add1D(1);
    scatterSampleOffset = sample->Add1D(1);
}


Spectrum VSDScatteringIntegrator::Transmittance(const Scene *scene,
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


Spectrum VSDScatteringIntegrator::LiSingle(const Scene *scene, const Renderer *renderer,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        Spectrum *T, MemoryArena &arena) const {
    VolumeRegion *vr = scene->volumeRegion;
    float t0, t1;
    vr->IntersectP(ray, &t0, &t1);

    // Do single scattering volume integration in _vr_
    Spectrum Lv(0.);

    // Prepare for volume integration stepping
    int nSamples = Ceil2Int((t1-t0) / stepSize);
    float step = (t1 - t0) / nSamples;
    Spectrum Tr(1.f);
    Point p = ray(t0), pPrev;
    t0 += sample->oneD[scatterSampleOffset][0] * step;

    // Compute the emission from the voxels, not from the light sources
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
        Lv += Tr * vr->PhotonDensity(p);
    }
    *T = Tr;
    return Lv * step;
}


Spectrum VSDScatteringIntegrator::LiMultiple(const Scene *scene, const Renderer *renderer,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        Spectrum *T, MemoryArena &arena) const {
    VolumeRegion *vr = scene->volumeRegion;
    float t0, t1;
    vr->IntersectP(ray, &t0, &t1);

    // Do multiple scattering volume integration in _vr_
    Spectrum Lv(0.);

    // Prepare for random walk
    Spectrum Tr(1.f);

    Point p = ray(t0), pPrev = ray.o;

    // printf("%f %f %f, ", p.x, p.y, p.z);

    int steps = 0;
    // Sample the events in a random walk

//    BBox bb = vr->WorldBound();
//    printf("%f %f %f & %f %f %f \n",
//           bb.pMin.x, bb.pMin.y, bb.pMin.z, bb.pMax.x, bb.pMax.y, bb.pMax.z);


    while(vr->WorldBound().Inside(p)) {
        //printf("%f %f %f %d \n", p.x, p.y, p.z, steps);
        steps++;
        // Sample direction
        Vector wi = -ray.d, wo;
        float pdfDirection = 0.f;
        if(!vr->SampleDirection(p, wi, wo, &pdfDirection, rng)) {
            // printf("return 1 \n");
            return Lv;
        }

        Ray r(p, wo, 0);

        // Sample a distance
        float pdfDistance = 0.f;
        float tDist;
        Point pSample;
        if(!vr->SampleDistance(r, &tDist, pSample, &pdfDistance, rng)) {
            // printf("return 2 \n");
            return Lv;
        }

        // Compute the transmittance
        pPrev = p;
        p = r(tDist);
        Ray tauRay(pPrev, p - pPrev, 0.f, 1.f, r.time, r.depth);
        Spectrum stepTau = vr->tau(tauRay, .5f * stepSize, rng.RandomFloat());
        Tr *= Exp(-stepTau);

        // Possibly terminate random walk if transmittance is small
        if (Tr.y() < 1e-3) {
            const float continueProb = .5f;
            if (rng.RandomFloat() > continueProb) {
                Tr = 0.f;
                // printf("return 3 \n");
                break;
            }
            Tr /= continueProb;
        }

        // Compute emission term at _p_
        Lv += Tr * vr->PhotonDensity(p) / (pdfDistance * pdfDirection);
    }

    // printf("%f %f %f %d \n", p.x, p.y, p.z, steps);

   // printf("%d, ", steps);
    return Lv;
}


Spectrum VSDScatteringIntegrator::Li(const Scene *scene, const Renderer *renderer,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        Spectrum *T, MemoryArena &arena) const {
    VolumeRegion *vr = scene->volumeRegion;
    float t0, t1;
    if (!vr || !vr->IntersectP(ray, &t0, &t1) || (t1-t0) == 0.f) {
        *T = 1.f;
        return 0.f;
    }

    Spectrum LSingle = LiSingle(scene, renderer, ray, sample, rng, T, arena);
    Spectrum LMultiple = LiMultiple(scene, renderer, ray, sample, rng, T, arena);
    return LSingle + LMultiple;
}


VSDScatteringIntegrator *CreateVSDScatteringIntegrator(const ParamSet &params) {
    float stepSize  = params.FindOneFloat("stepsize", 1.f);
    return new VSDScatteringIntegrator(stepSize);
}

}
