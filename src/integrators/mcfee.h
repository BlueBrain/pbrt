
/*
    pbrt source code Copyright(c) 2015-2018 Marwan Abdellah
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

#ifndef PBRT_INTEGRATORS_MCFEE_H
#define PBRT_INTEGRATORS_MCFEE_H

// integrators/mcfee.h*
#include "volume.h"
#include "integrator.h"
#include "shapes/bead.h"
#include <vsd/vsdsprite.h>
#include <vector>
#include <iostream>
#include <shapes/sphere.h>


// MCFEE Declarations
class MCFEE : public VolumeIntegrator {
public:
    // MCFEE Public Methods
    MCFEE(const uint64_t numPhotons) {
        numberPhotons = numPhotons;
        stepSize = 0.1f;
        photonState = EXCITATION;
    }
    void Preprocess(const Scene *, const Camera *, const Renderer *);
    void RunSimulationWithSingleBead(const Scene *scene, Bead *bead);
    void RunSimulationWithMultipleBeads(const Scene *scene);
    bool ExcitationPath(const Scene *scene, Light *fiber, RNG &rng, Point &hitPoint);
    void EmissionPath(const Scene *scene, RNG &rng, const Point &hitPoint);


    void PhotonPacketRandomWalk(const Scene *scene, FluorescentEvent &event, RNG& rng);
    void PhotonRandomWalk(const Scene *scene, FluorescentEvent &event, RNG& rng);
    Spectrum Transmittance(const Scene *, const Renderer *,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        MemoryArena &arena) const;
    void RequestSamples(Sampler *sampler, Sample *sample,
        const Scene *scene);
    Spectrum Li(const Scene *, const Renderer *, const RayDifferential &ray,
         const Sample *sample, RNG &rng, Spectrum *T, MemoryArena &arena) const;
public:
    // MCFEE Prublic Enumerators
    enum PHOTON_STATE {
        EXCITATION,
        EMISSION
    };
private:
    // MCFEE Private Data
    int tauSampleOffset, scatterSampleOffset;
    float stepSize;
    PHOTON_STATE photonState; // Photon state
    uint64_t numberPhotons; // Number of photons from the fiber
};

MCFEE* CreateMCFEE(const ParamSet &params);

#endif // PBRT_INTEGRATORS_MCFEE_H
