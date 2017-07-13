
/*
    pbrt source code Copyright(c) 2015-2016 Marwan Abdellah
                                  <marwan.abdellah@epfl.ch>

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

#ifndef PBRT_INTEGRATORS_VSD_FLS_H
#define PBRT_INTEGRATORS_VSD_FLS_H

// integrators/vsdfds.h*
#include "volume.h"
#include "integrator.h"
#include <vsd/vsdsprite.h>
#include <vector>
#include <iostream>

// VSDForwardLinearIntegrator Declarations
class VSDForwardLinearIntegrator : public VolumeIntegrator {
public:
    // VSDForwardLinearIntegrator Public Methods
    VSDForwardLinearIntegrator(const std::string datadirectory,
                              const std::string pshfile,
                              const Vector &shift) {
        // Read the sprite and shift it to reside under the sensor
        sprite.Read(datadirectory, pshfile, false, shift);
        stepSize = 0.1;
    }
    void Preprocess(const Scene *, const Camera *, const Renderer *);
    void PhotonPacketRandomWalk(const Scene *scene, FluorescentEvent &event, RNG& rng);
    void PhotonRandomWalk(const Scene *scene, FluorescentEvent &event, RNG& rng);
    Spectrum Transmittance(const Scene *, const Renderer *,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        MemoryArena &arena) const;
    void RequestSamples(Sampler *sampler, Sample *sample,
        const Scene *scene);
    Spectrum Li(const Scene *, const Renderer *, const RayDifferential &ray,
         const Sample *sample, RNG &rng, Spectrum *T, MemoryArena &arena) const;
private:
    // VSDForwardLinearIntegrator Private Data
    int tauSampleOffset, scatterSampleOffset;
    float stepSize;
    VSDSprite sprite;
    Vector shift;
};

VSDForwardLinearIntegrator *CreateVSDForwardLinearIntegrator(const ParamSet &params);

#endif // PBRT_INTEGRATORS_VSD_FLS_H
