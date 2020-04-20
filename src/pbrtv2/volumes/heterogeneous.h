
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

#ifndef PBRT_VOLUMES_HETEROGENEOUS_H
#define PBRT_VOLUMES_HETEROGENEOUS_H

// volumes/heterogeneous.h*
#include "volume.h"
#include "montecarlo.h"

namespace pbrt
{

// HeterogeneousVolumeDensity Declarations
class HeterogeneousVolumeDensity : public VolumeRegion {
public:
    // HeterogeneousVolumeDensity Public Methods
    HeterogeneousVolumeDensity(const Spectrum &sa, const Spectrum &ss,
        const float mind, const float maxd, float gg, const Spectrum &em, const BBox &e,
        const Transform &v2w) {
        WorldToVolume = Inverse(v2w);
        sigma_a = sa; sigma_s = ss; g = gg;
        le = em;
        extent = e;
        minDensity = mind;
        maxDensity = maxd;
    }

    // Pre-processing
    void PreProcess() {}

    // Geometry
    BBox WorldBound() const { return Inverse(WorldToVolume)(extent); }
    bool IntersectP(const Ray &r, float *t0, float *t1) const {
        Ray ray = WorldToVolume(r);
        return extent.IntersectP(ray, t0, t1);
    }
    bool SkipEmptySpace(const Ray &r, float *tSkip) const {
        Ray ray = WorldToVolume(r);
        float t0, t1;
        if(IntersectP(ray, &t0, &t1)) {
            *tSkip = t0;
            return true;
        }
        return false;
    }

    // Data
    float RandomDensity() const {
        return minDensity + static_cast <float> (rand()) /
                ( static_cast <float> (RAND_MAX/(maxDensity - minDensity)));
    }
    float Density(const Point &Pobj) const {
        if (extent.Inside(Pobj)) return RandomDensity();
        return 0.f;
    }
    float PhotonDensity(const Point &p) const {
        const Point Pobj = WorldToVolume(p);
        return Density(Pobj);
    }

    // Spectral Optical Properties
    Spectrum Sigma_a(const Point &p, const Vector &, float) const {
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ? (sigma_a * Density(Pobj)) : 0.;
    }
    Spectrum Sigma_af(const Point &p, const Vector &, float) const {
        return  0.f;
    }
    Spectrum Sigma_s(const Point &p, const Vector &, float) const {
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ? (sigma_s * Density(Pobj)) : 0.;
    }
    Spectrum Sigma_t(const Point &p, const Vector &, float) const {
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ?
                    ((sigma_a + sigma_s) * Density(Pobj)) : 0.f;
    }
    Spectrum MaxSigma_t() const { return RandomDensity() * (sigma_a + sigma_s); }
    Spectrum STER(const Point &p, const Vector &w, float time) const {
            return (Sigma_s(p, w, time) / Sigma_t(p, w, time));
    }
    Spectrum ATER(const Point &p, const Vector &w, float time) const {
            return (Sigma_a(p, w, time) / Sigma_t(p, w, time));
    }
    Spectrum Lve(const Point &p, const Vector &, float) const {
        return extent.Inside(WorldToVolume(p)) ? le : 0.;
    }
    float p(const Point &p, const Vector &wi, const Vector &wo, float) const {
        if (!extent.Inside(WorldToVolume(p))) return 0.;
        return PhaseHG(wi, wo, g);
    }
    Spectrum tau(const Ray &ray, float, float) const {
        float t0, t1;
        if (!IntersectP(ray, &t0, &t1)) return 0.;
        // Optical thicknees in Beer-lambert law
        return Distance(ray(t0), ray(t1)) * ((sigma_a + sigma_s) * RandomDensity());
    }

    // Optical Properties at Specific Wavelength _wl_
    float Sigma_a(const Point &p, const Vector &, float,
        const int &wl) const {
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ? (sigma_a.Power(wl) * Density(Pobj)) : 0.;
    }
    float Sigma_af(const Point &p, const Vector &, float,
        const int &wl) const { return  0.f; }
    float Sigma_s(const Point &p, const Vector &, float,
        const int &wl) const {
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ? (sigma_s.Power(wl) * Density(Pobj)) : 0.;
    }
    float Sigma_t(const Point &p, const Vector &, float,
        const int &wl) const {
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ? ((sigma_a.Power(wl) + sigma_s.Power(wl))
                                      * Density(Pobj)) : 0.f;
    }
    float STER(const Point &p, const Vector &w, float time,
        const int &wl) const {
            return (Sigma_s(p, w, time, wl) / Sigma_t(p, w, time, wl));
    }
    float ATER(const Point &p, const Vector &w, float time,
        const int &wl) const {
            return (Sigma_a(p, w, time, wl) / Sigma_t(p, w, time, wl));
    }
    float Lve(const Point &p, const Vector &, float, const int &wl) const {
        return extent.Inside(WorldToVolume(p)) ? le.Power(wl) : 0.;
    }
    float MaxSigma_t(const int &wl) const {
        return RandomDensity() * (sigma_a.Power(wl) + sigma_s.Power(wl));
    }
    float tauLambda(const Ray &ray, float, float, const int &wl) const {
        float t0, t1;
        if (!IntersectP(ray, &t0, &t1)) return 0.;
        // Optical thikness in Beer-lambert law at _wl_
        return Distance(ray(t0), ray(t1)) *
                ((sigma_a.Power(wl) + sigma_s.Power(wl)) * RandomDensity());
    }

    // Medium Sampling
    bool SampleDistance(const Ray& ray, float* tDist, Point &Psample,
        float* pdf, RNG &rng) const;
    bool SampleDirection(const Point &p, const Vector& wi, Vector& wo,
        float* pdf, RNG &rng) const;

    // Medium Sampling at Specific Wavelength _wl_
    bool SampleDistance(const Ray& ray, float* tDist, Point &Psample,
        float* pdf, RNG &rng, const int &wl) const;

private:
    // HeterogeneousVolumeDensity Private Data
    Spectrum sigma_a, sigma_s, le;
    float g;
    BBox extent;
    Transform WorldToVolume;
    float minDensity, maxDensity;
};


HeterogeneousVolumeDensity *CreateHeterogeneousVolumeDensityRegion
(const Transform &volume2world, const ParamSet &params);
}
#endif // PBRT_VOLUMES_HETEROGENEOUS_H
