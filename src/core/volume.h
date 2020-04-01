
/*
    pbrt source code Copyright(c) 1998-2012 Matt Pharr and Greg Humphreys.
                                  2012-2015 Marwan Abdellah.

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

#ifndef PBRT_CORE_VOLUME_H
#define PBRT_CORE_VOLUME_H

// core/volume.h*
#include "pbrt.h"
#include "spectrum.h"
#include "geometry.h"
#include "transform.h"
#include "integrator.h"

namespace pbrt
{

// Volume Scattering Declarations
float PhaseIsotropic(const Vector &w, const Vector &wp);
float PhaseRayleigh(const Vector &w, const Vector &wp);
float PhaseMieHazy(const Vector &w, const Vector &wp);
float PhaseMieMurky(const Vector &w, const Vector &wp);
float PhaseHG(const Vector &w, const Vector &wp, float g);
float PhaseSchlick(const Vector &w, const Vector &wp, float g);


class MediumSamplingRecord {
public:
    // MediumSamplingRecord Declarations
    float t;
    float p;
    Vector w;
    Spectrum mu_a;
    Spectrum mu_s;
    Spectrum Tr;
    float pdfSuccess;
    float pdfFailure;
};

struct PathPoint {
    Point p;
    uint64_t bounceIdx;
};


struct BinaryFluorescentVoxel {
    bool density;
    bool fluorescence;
};


struct  FluorescentVoxel {
    unsigned char density;
    bool fluorescence;
    uint64_t fIndex;
};


// VolumePathIntegrator Local Structures
struct VolumeVertex {
    VolumeVertex() { }
    VolumeVertex(const Point &pp, const Vector &wii,
                 const Spectrum &ssa, const  Spectrum &sss, const Spectrum &c,
                 const float &w)
        : p(pp), wi(wii), sa(ssa), ss(sss), pathContrib(c), weight(w) { }
    Point p;
    Vector wi;
    Spectrum sa;
    Spectrum ss;
    Spectrum pathContrib;
    float weight;
};

typedef std::vector<VolumeVertex> VolumeVertexList;

class VolumeRegion {
public:
    // VolumeRegion Interface
    virtual ~VolumeRegion();
    virtual void PreProcess();
    virtual void ValidateData() const;
    virtual bool IsFluorescent() const;
    virtual bool HasNonClearedFluorescentVolumes() const;
    virtual BBox WorldBound() const = 0;
    virtual bool IntersectP(const Ray &ray, float *t0, float *t1) const = 0;
    virtual bool SkipEmptySpace(const Ray &r, float *tSkip) const;
    virtual float Density(const Point &Pobj) const { return 0.f;}
    virtual float PhotonDensity(const Point &p) const;
    virtual float Fluorescence(const Point &p) const;
    virtual Spectrum Mu(const Point &p, const Vector &wo, float t) const;
    virtual Spectrum Sigma_a(const Point &p, const Vector &wo,
                float t) const = 0;
    virtual Spectrum Sigma_af(const Point &p, const Vector &wo,
                float t) const;
    virtual Spectrum Sigma_s(const Point &p, const Vector &wo,
                float t) const = 0;
    virtual Spectrum Sigma_t(const Point &p, const Vector &wo, float t) const;
    virtual Spectrum STER(const Point &p, const Vector &wo, float t) const;
    virtual Spectrum ATER(const Point &p, const Vector &wo,
                float t) const;
    virtual Spectrum MaxSigma_t() const;
    virtual Spectrum Lve(const Point &p, const Vector &wo, float t) const;
    virtual Spectrum tau(const Ray &r, float step = 1.f,
                float offset = 0.5) const = 0;
    virtual float Mu(const Point &p, const Vector &wo, float t,
                const int &wl) const;
    virtual float Sigma_a(const Point &p, const Vector &wo, float t,
                const int &wl) const = 0;
    virtual float Sigma_af(const Point &p, const Vector &wo, float t,
                const int &wl) const;
    virtual float Sigma_s(const Point &p, const Vector &wo, float t,
                const int &wl) const = 0;
    virtual float Sigma_t(const Point &p, const Vector &wo, float t,
                const int &wl) const;
    virtual float MaxSigma_t(const int &wl) const;
    virtual float STER(const Point &p, const Vector &wo, float t,
                const int &wl) const;
    virtual float ATER(const Point &p, const Vector &wo, float t,
                const int &wl) const;
    virtual float Lve(const Point &p, const Vector &wo, float t,
                const int &wl) const;
    virtual float tauLambda(const Ray &r, float step = 1.f, float offset = 0.5,
                const int &wl = 0) const = 0;
    virtual float p(const Point &p, const Vector &wi, const Vector &wo,
                float t) const;
    virtual float pf(const Point &p, const Vector &wi, const Vector &wo,
                float t) const;
    virtual Spectrum fEx(const Point &p) const;
    virtual Spectrum fEm(const Point &p) const;
    virtual float fEx(const Point &p, const int &wl) const;
    virtual float fEm(const Point &p, const int &wl) const;
    virtual float Yeild(const Point &p) const;
    virtual bool SampleDistance(const Ray &r, float *tDis, Point &Psample,
                float *pdf, RNG &rng) const;
    virtual bool SampleDirection(const Point &p, const Vector &wi, Vector &wo,
                float *pdf, RNG &rng) const;
    virtual bool SampleDistance(const Ray &r, float *tDis, Point &Psample,
                float *pdf, RNG &rng, const int &wl) const;
};


class DensityRegion : public VolumeRegion {
public:
    // DensityRegion Public Methods
    DensityRegion(const Spectrum &sa, const Spectrum &ss, float gg,
            const Spectrum &em, const Transform &v2w)
        : sigma_a(sa), sigma_s(ss), le(em), g(gg),
          WorldToVolume(Inverse(v2w)) { }
    Spectrum Sigma_a(const Point &p, const Vector &, float) const {
        return Density(WorldToVolume(p)) * sigma_a;
    }
    Spectrum Sigma_s(const Point &p, const Vector &, float) const {
        return Density(WorldToVolume(p)) * sigma_s;
    }
    Spectrum Sigma_t(const Point &p, const Vector &, float) const {
        return Density(WorldToVolume(p)) * (sigma_a + sigma_s);
    }
    Spectrum STER(const Point &p, const Vector &w, float time) const {
        return (Sigma_s(p, w, time) / Sigma_t(p, w, time));
    }
    Spectrum ATER(const Point &p, const Vector &w, float time) const {
        return (Sigma_a(p, w, time) / Sigma_t(p, w, time));
    }
    Spectrum Lve(const Point &p, const Vector &, float) const {
        return Density(WorldToVolume(p)) * le;
    }
    float p(const Point &p, const Vector &w, const Vector &wp, float) const {
        return PhaseHG(w, wp, g);
    }
    Spectrum tau(const Ray &r, float stepSize, float offset) const;
    float Sigma_a(const Point &p, const Vector &, float,
        const int &wl) const {
        return Density(WorldToVolume(p)) * sigma_a.Power(wl);
    }
    float Sigma_s(const Point &p, const Vector &, float,
        const int &wl) const {
        return Density(WorldToVolume(p)) * sigma_s.Power(wl);
    }
    float Sigma_t(const Point &p, const Vector &, float,
        const int &wl) const {
        return Density(WorldToVolume(p)) * (sigma_a.Power(wl) + sigma_s.Power(wl));
    }
    float STER(const Point &p, const Vector &w, float time,
        const int &wl) const {
        return (Sigma_s(p, w, time, wl) / Sigma_t(p, w, time, wl));
    }
    float ATER(const Point &p, const Vector &w, float time,
        const int &wl) const {
        return (Sigma_a(p, w, time, wl) / Sigma_t(p, w, time, wl));
    }
    float Lve(const Point &p, const Vector &, float,
        const int &wl) const {
        return Density(WorldToVolume(p)) * le.Power(wl);
    }
    float tauLambda(const Ray &r, float stepSize, float offset,
        const int &wl) const;
protected:
    // DensityRegion Protected Data
    Spectrum sigma_a, sigma_s, sigma_t, le;
    float g;
    Transform WorldToVolume;
};


class AggregateVolume : public VolumeRegion {
public:
    // AggregateVolume Public Methods
    AggregateVolume(const vector<VolumeRegion *> &r);
    ~AggregateVolume();
    BBox WorldBound() const;
    bool IntersectP(const Ray &ray, float *t0, float *t1) const;
    bool SkipEmptySpace(const Ray &ray, float *tSkip) const;
    float Density(const Point &Pobj) const;
    float PhotonDensity(const Point &p) const;
    bool HasNonClearedFluorescentVolumes() const;
    Spectrum Mu(const Point &, const Vector &, float) const;
    Spectrum Sigma_a(const Point &, const Vector &, float) const;
    Spectrum Sigma_af(const Point &, const Vector &, float) const;
    Spectrum Sigma_s(const Point &, const Vector &, float) const;
    Spectrum Sigma_t(const Point &, const Vector &, float) const;
    Spectrum MaxSigma_t() const;
    Spectrum STER(const Point &p, const Vector &wo, float t) const;
    Spectrum ATER(const Point &p, const Vector &wo, float t) const;
    Spectrum Lve(const Point &, const Vector &, float) const;
    float p(const Point &, const Vector &, const Vector &, float) const;
    Spectrum tau(const Ray &ray, float, float) const;
    float Mu(const Point &, const Vector &, float, const int &wl) const;
    float Sigma_a(const Point &, const Vector &, float, const int &wl) const;
    float Sigma_af(const Point &, const Vector &, float, const int &wl) const;
    float Sigma_s(const Point &, const Vector &, float, const int &wl) const;
    float Sigma_t(const Point &, const Vector &, float, const int &wl) const;
    float MaxSigma_t(const int &wl) const;
    float STER(const Point &p, const Vector &wo, float t, const int &wl) const;
    float ATER(const Point &p, const Vector &wo, float t, const int &wl) const;
    float Lve(const Point &, const Vector &, float, const int &wl) const;
    float tauLambda(const Ray &ray, float, float, const int &wl) const;
    float Fluorescence(const Point &Pobj) const;
    Spectrum fEx(const Point &p) const;
    Spectrum fEm(const Point &p) const;
    float Yeild(const Point &p) const;
    float pf(const Point &, const Vector &, const Vector &, float) const;
    float fEx(const Point &p, const int &wl) const;
    float fEm(const Point &p, const int &wl) const;
    bool SampleDistance(const Ray& ray, float* tDis, Point &Psample,
            float* pdf, RNG &rng) const;
    bool SampleDirection(const Point& p, const Vector& wi, Vector& wo,
            float* pdf, RNG &rng) const;
    bool SampleDistance(const Ray& ray, float* tDis, Point &Psample,
            float* pdf, RNG &rng, const int &wl) const;
private:
    // AggregateVolume Private Data
    vector<VolumeRegion *> regions;
    BBox bound;
};


bool GetVolumeScatteringProperties(const string &name, Spectrum *sigma_a,
                                   Spectrum *sigma_prime_s);


class VolumeIntegrator : public Integrator {
public:
    // VolumeIntegrator Interface
    virtual Spectrum Li(const Scene *scene, const Renderer *renderer,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        Spectrum *transmittance, MemoryArena &arena) const = 0;
    virtual Spectrum Transmittance(const Scene *scene,
        const Renderer *renderer, const RayDifferential &ray,
        const Sample *sample, RNG &rng, MemoryArena &arena) const = 0;
};


void SubsurfaceFromDiffuse(const Spectrum &Kd, float meanPathLength, float eta,
        Spectrum *sigma_a, Spectrum *sigma_prime_s);

}

#endif // PBRT_CORE_VOLUME_H
