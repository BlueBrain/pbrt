
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

#ifndef PBRT_INTEGRATORS_SENSOR_H
#define PBRT_INTEGRATORS_SENSOR_H

// integrators/sensor.h*
#include "volume.h"
#include "integrator.h"

namespace pbrt
{

struct Photon {
    Spectrum L;
    Ray ray;
};

// SensorIntegrator Declarations
class SensorIntegrator : public VolumeIntegrator {
public:
    // SensorIntegrator Public Methods
    SensorIntegrator(float ss, uint64_t photons) {
        stepSize = ss;
        photonCount = photons;
    }
    void Preprocess(const Scene *, const Camera *, const Renderer *);
    void VolumeRandomWalk(const Scene *scene, Photon& photon, RNG& rng);
    void PhotonRandomWalk(const Scene *scene, Photon& photon, RNG& rng);
    Spectrum Transmittance(const Scene *, const Renderer *,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        MemoryArena &arena) const;
    void RequestSamples(Sampler *sampler, Sample *sample,
        const Scene *scene);
    Spectrum Li(const Scene *, const Renderer *, const RayDifferential &ray,
         const Sample *sample, RNG &rng, Spectrum *T, MemoryArena &arena) const;
    bool SamplePhoton(const Scene *scene, RNG &rng, Photon& photon, Normal* normal);

private:
    // SensorIntegrator Private Data
    float stepSize;
    uint64_t photonCount;
    int tauSampleOffset, scatterSampleOffset;
};


SensorIntegrator *CreateSensorIntegrator (const ParamSet &params);

}

#endif // PBRT_INTEGRATORS_SINGLE_H
