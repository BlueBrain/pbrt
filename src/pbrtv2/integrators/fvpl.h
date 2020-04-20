
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

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PBRT_INTEGRATORS_FVPL_H
#define PBRT_INTEGRATORS_FVPL_H

// integrators/fvpl.h*
#include "volume.h"
#include "integrator.h"

namespace pbrt
{

struct VPL {
    VPL(const Point &pp,  const Spectrum &c, const Vector &wi)
        : p(pp), pathContrib(c), w(wi) { }
    Point p;
    Spectrum pathContrib;
    Vector w;
};

// FVPLIntegrator Declarations
class FVPLIntegrator : public VolumeIntegrator {
public:
    // FVPLIntegrator Public Methods
    FVPLIntegrator(float ss, uint32_t nl, uint32_t ns, float gl) {
        stepSize = ss;
        nLightPaths = RoundUpPow2(nl);
        nLightSets = RoundUpPow2(ns);
        vpls.resize(nLightSets);
        gLimit = gl;
    }
    void Preprocess(const Scene *scene, const Camera *camera,
        const Renderer *renderer);
    void WriteVPLs();
    Spectrum Transmittance(const Scene *, const Renderer *,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        MemoryArena &arena) const;
    Spectrum Transmittance(const Scene *scene, const Point &p1,
        const Point &p2, RNG rng) const;
    void RequestSamples(Sampler *sampler, Sample *sample,
        const Scene *scene);
    Spectrum Li(const Scene *, const Renderer *, const RayDifferential &ray,
         const Sample *sample, RNG &rng, Spectrum *T, MemoryArena &arena) const;
private:
    // FVPLIntegrator Private Data
    int minLambda;
    int maxLambda;
    float stepSize;
    float gLimit;
    int tauSampleOffset, scatterSampleOffset;
    LightSampleOffsets *lightSampleOffsets;
    int vlSetOffset;
    uint32_t nLightPaths, nLightSets;
    vector<vector<VPL> > vpls;
};

FVPLIntegrator *CreateFVPLIntegrator(const ParamSet &params);

}

#endif // PBRT_INTEGRATORS_FVPL_H
