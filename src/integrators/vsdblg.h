
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

#ifndef PBRT_INTEGRATORS_VSD_BLG_H
#define PBRT_INTEGRATORS_VSD_BLG_H

// integrators/vsdblg.h*
#include "volume.h"
#include "integrator.h"

namespace pbrt
{

// VSDBackwardLinearIntegrator Declarations
class VSDBackwardLinearIntegrator : public VolumeIntegrator {
public:
    // VSDBackwardLinearIntegrator Public Methods
    VSDBackwardLinearIntegrator(float ss) { stepSize = ss; }
    Spectrum Transmittance(const Scene *, const Renderer *,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        MemoryArena &arena) const;
    void RequestSamples(Sampler *sampler, Sample *sample,
        const Scene *scene);
    Spectrum Li(const Scene *, const Renderer *, const RayDifferential &ray,
         const Sample *sample, RNG &rng, Spectrum *T, MemoryArena &arena) const;
private:
    // VSDBackwardLinearIntegrator Private Data
    float stepSize;
    int tauSampleOffset, scatterSampleOffset;
};


VSDBackwardLinearIntegrator *CreateVSDBackwardLinearIntegrator(const ParamSet &params);

}

#endif // PBRT_INTEGRATORS_VSD_BLG_H
