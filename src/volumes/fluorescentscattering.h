
/*
    pbrt source code Copyright(c) 1998-2012 Matt Pharr and Greg Humphreys.
                                  2012-2016 Marwan Abdellah.

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

#ifndef PBRT_VOLUMES_FLUORESCENTSCATTERING_H
#define PBRT_VOLUMES_FLUORESCENTSCATTERING_H

// volumes/fluorescentscattering.h*
#include "volume.h"
#include "montecarlo.h"

namespace pbrt
{

// FluorescentScatteringVolumeDensity Declarations
class FluorescentScatteringVolumeDensity : public VolumeRegion {
public:

    // FluorescentScatteringVolumeDensity Public Methods
    FluorescentScatteringVolumeDensity(const Spectrum &sa, const Spectrum &ss, float gg,
            const Spectrum &em, const BBox &e, const Transform &v2w,
            const Spectrum &fex = 0.f, const Spectrum &fem = 0.f,
            float molecularweight = 0.f, float epsilon_moles = 0.f,
            float conc_gramperliter = 0.f, float qyield = 0.f, float ggf = 0.f,
            float sscale = 1.f, float fscale = 1.f, float d = 1.0) {
        WorldToVolume = Inverse(v2w);

        yieldf = qyield;
        density = d; g = gg; gf = ggf;
        s_scale = sscale; f_scale = fscale;
        le = em;
        extent = e;

        // Fluorescence spectra
        f_ex = fex * f_scale ; f_em = fem * f_scale;
        f_ex.Normalize();
        f_em.Normalize();

        // Compute the concentration in [/M /cm] from [g/l]
        // C[/M /cm] = C[g/l] / Molecular Weight [kDa]
        molecwt = molecularweight;
        epsilon = epsilon_moles;
        c = conc_gramperliter / (1000 * molecwt);

        // Optical properties
        // Compute the fluorescence absorption coefficient spectrum
        // sigma_af = ln(10) * C[/M /cm] * epsilon[M]
        sigma_af = (LN10 * c * epsilon * f_ex) * s_scale;
        sigma_anf = sa * s_scale;
        sigma_a = sigma_anf + sigma_af;
        sigma_s = ss * s_scale;
        sigma_t = sigma_a + sigma_s;

        ValidateData();
    }

    void ValidateData() const;

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
    float Density(const Point &Pobj) const {
        if (extent.Inside(Pobj)) return density;
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
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ? (sigma_af * Density(Pobj)) : 0.;
    }
    Spectrum Sigma_s(const Point &p, const Vector &, float) const {
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ? (sigma_s * Density(Pobj)) : 0.;
    }
    Spectrum Sigma_sf(const Point &p, const Vector &, float) const {
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ? (sigma_sf * Density(Pobj)) : 0.;
    }
    Spectrum Sigma_t(const Point &p, const Vector &, float) const {
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ? (sigma_t * Density(Pobj)) : 0.f;
    }
    Spectrum MaxSigma_t() const {
        return density * s_scale * (sigma_t);
    }
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
        return Distance(ray(t0), ray(t1)) * (sigma_t * density);
    }

    // Optical Properties at Specific Wavelength _wl_
    float Sigma_a(const Point &p, const Vector &, float, const int &wl) const {
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ? (sigma_a.Power(wl) * Density(Pobj)) : 0.;
    }
    float Sigma_af(const Point &p, const Vector &, float, const int &wl) const {
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ? (sigma_af.Power(wl) * Density(Pobj)) : 0.;
    }
    float Sigma_s(const Point &p, const Vector &, float, const int &wl) const {
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ? (sigma_s.Power(wl) * Density(Pobj)) : 0.;
    }
    float Sigma_sf(const Point &p, const Vector &, float, const int &wl) const {
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ? (sigma_sf.Power(wl) * Density(Pobj)) : 0.;
    }
    float Sigma_t(const Point &p, const Vector &, float, const int &wl) const {
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ? sigma_t.Power(wl) * Density(Pobj) : 0.f;
    }
    float MaxSigma_t(const int &wl) const {
        return density * s_scale *
                (sigma_a.Power(wl) + sigma_af.Power(wl) + sigma_s.Power(wl));
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
    float tauLambda(const Ray &ray, float, float, const int &wl) const {
        float t0, t1;
        if (!IntersectP(ray, &t0, &t1)) return 0.;
        const float sigma_t = density * s_scale *
                (sigma_a.Power(wl) + sigma_s.Power(wl));
        return Distance(ray(t0), ray(t1)) * (sigma_t);
    }

    // Fluorescence
    bool IsFluorescent() const { return true; }
    float Fluorescence(const Point &Pobj) const {
        if (extent.Inside(Pobj)) return 1;
        return 0;
    }
    Spectrum fEx(const Point &p) const { return f_ex; }
    Spectrum fEm(const Point &p) const { return f_em; }
    float Yeild(const Point &p) const { return yieldf; }
    float pf(const Point &p, const Vector &wi, const Vector &wo, float) const {
        if (!extent.Inside(WorldToVolume(p))) return 0.;
        return PhaseHG(wi, wo, gf);
    }

    // Fluorescence at Specific Wavelength _wl_
    float fEx(const Point &p, const int &wl) const { return f_ex.Power(wl); }
    float fEm(const Point &p, const int &wl) const { return f_em.Power(wl); }

    // Medium Sampling
    bool SampleDistance(const Ray& ray, float* tDist, Point &Psample,
        float* pdf, RNG &rng) const;
    bool SampleDirection(const Point &p, const Vector& wi, Vector& wo,
        float* pdf, RNG &rng) const;

    // Medium Sampling at Specific Wavelength _wl_
    bool SampleDistance(const Ray& ray, float* tDist, Point &Psample,
        float* pdf, RNG &rng, const int &wl) const;

private:
    // FluorescentScatteringVolumeDensity Private Data
    Spectrum sigma_anf, sigma_af, sigma_a, sigma_s, sigma_sf, sigma_t, le;
    Spectrum f_ex, f_em;
    float molecwt, epsilon, c, yieldf, s_scale, f_scale, g, gf;
    BBox extent;
    Transform WorldToVolume;
    float density;
};


FluorescentScatteringVolumeDensity *CreateFluorescentScatteringVolumeDensityRegion
(const Transform &volume2world, const ParamSet &params);


}
#endif // PBRT_VOLUMES_FLUORESCENTSCATTERING_H
