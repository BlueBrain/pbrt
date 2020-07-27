
/*
    pbrt source code Copyright(c) 2015-2017 Marwan Abdellah
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

#ifndef PBRT_INTEGRATORS_MONTECARLO_FLUORESECFNCE_H
#define PBRT_INTEGRATORS_MONTECARLO_FLUORESECFNCE_H

// integrators/montecarlofluorescence.h*
#include "volume.h"
#include "integrator.h"
#include <vsd/vsdsprite.h>
#include <vector>
#include <iostream>

// MonteCarloFluorescenceIntegrator Declarations
class MonteCarloFluorescenceIntegrator : public VolumeIntegrator {
public:
    // MonteCarloFluorescenceIntegrator Public Methods
    MonteCarloFluorescenceIntegrator(const Point &beadPos,
                                     const uint64_t numPhotons) {
        beadPosition = beadPos;
        numberPhotons = numPhotons;
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
    // MonteCarloFluorescenceIntegrator Private Data
    int tauSampleOffset, scatterSampleOffset;
    float stepSize;

    Point beadPosition; // Bead position in XYZ
    uint64_t numberPhotons; // Number of photons launched from the bead
};

MonteCarloFluorescenceIntegrator*
CreateMonteCarloFluorescenceIntegrator(const ParamSet &params);

#endif // PBRT_INTEGRATORS_MONTECARLO_FLUORESECFNCE_H
