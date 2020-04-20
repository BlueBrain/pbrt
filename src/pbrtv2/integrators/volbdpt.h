
/*
    pbrt source code Copyright(c) 1998-2012 Matt Pharr and Greg Humphreys.
                                  2015 Marwan Abdellah

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

#ifndef PBRT_INTEGRATORS_VOLUME_BDPT_H
#define PBRT_INTEGRATORS_VOLUME_BDPT_H

// integrators/volpath.h*
#include "volume.h"
#include "integrator.h"
#include <vector>

namespace pbrt
{

// VolumePathIntegrator Declarations
class VolumeBDPTIntegrator : public VolumeIntegrator {
public:
    // VolumePathIntegrator Public Methods
    VolumeBDPTIntegrator(float ss, uint64_t eyeBounces, uint64_t lightBounces) {
        stepSize = ss;
        maxEyeDepth = eyeBounces;
        maxLightDepth = lightBounces;
    }
    Spectrum Transmittance(const Scene *, const Renderer *,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        MemoryArena &arena) const;
    void RequestSamples(Sampler *sampler, Sample *sample,
        const Scene *scene);
    void Preprocess(const Scene *scene, const Camera *camera, const Renderer *renderer);
    void EyeRandomWalk(const Scene *scene, const Ray &eyeRay, VolumeVertexList& vList,
        RNG &rng) const;
    void LightRandomWalk(const Scene *scene, VolumeVertexList& vList, RNG &rng) const;
    Spectrum EvaluatePath(const Scene *scene, const VolumeVertexList &eyePath,
        const VolumeVertexList &lightPath, const uint64_t nEye, const uint64_t nLight, RNG &rng) const;
    // Single scattering contribution
    Spectrum Li_Single(const Scene *, const Renderer *, const RayDifferential &ray,
         const Sample *sample, RNG &rng, Spectrum *T, MemoryArena &arena) const;
    // Multiple scattering contribution
    Spectrum Li_Multiple(const Scene *, const Renderer *, const RayDifferential &r,
         const Sample *sample, RNG &rng, Spectrum *T, MemoryArena &arena) const;
    Spectrum Li(const Scene *, const Renderer *, const RayDifferential &ray,
         const Sample *sample, RNG &rng, Spectrum *T, MemoryArena &arena) const;
    Spectrum EstimateDirectLight(const Scene *scene, const Renderer *renderer,
        MemoryArena &arena, const Light *light, const Point &p,
        const Normal &n, const Vector &wo, float rayEpsilon, float time,
        RNG &rng, const LightSample &lightSample) const;
    Spectrum UniformSampleLight(const Scene *scene, const Renderer *renderer,
        MemoryArena &arena, const Point &p, const Normal &n, const Vector &wo,
        float rayEpsilon, float time, RNG &rng) const;
private:
    // VolumePathIntegrator Private Data
    float stepSize;
    int tauSampleOffset, scatterSampleOffset;
    uint64_t maxEyeDepth;
    uint64_t maxLightDepth;
};


VolumeBDPTIntegrator *CreateVolumeBDPTIntegrator(const ParamSet &params);

}

#endif // PBRT_INTEGRATORS_VOLUME_BDPT_H
