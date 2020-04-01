
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

#ifndef PBRT_VOLUMES_FLUORESCENTSCATTERINGVOLUMEGRID_H
#define PBRT_VOLUMES_FLUORESCENTSCATTERINGVOLUMEGRID_H

// volumes/fluorescentscatteringgrid.h*
#include "volume.h"

namespace pbrt
{

// FluorescentScatteringGridDensity Declarations
class FluorescentScatteringGridDensity : public DensityRegion {
public:
    // FluorescentScatteringGridDensity Public Methods
    FluorescentScatteringGridDensity(const Spectrum &sa, const Spectrum &ss,
            float gg, const Spectrum &em, const BBox &e, const Transform &v2w,
            int x, int y, int z, const float *d,
            const Spectrum &fex = 0.f, const Spectrum &fem = 0.f,
            float eps = 0.f, float conc = 0.f, float phi = 0.f, float ggf = 0.f)
        : DensityRegion(sa, ss, gg, em, v2w), nx(x), ny(y), nz(z),
            extent(e), yield(phi), gf(ggf) {
        fluorescenceDensity = new float[nx*ny*nz];
        memcpy(fluorescenceDensity, d, nx*ny*nz*sizeof(float));
        f_ex = fex; f_em = fem;
        f_ex.Normalize(); f_em.Normalize();
        epsilon = eps; c = conc;
        sigma_af = (LN10 * c * epsilon * f_ex);
        sigma_anf = sa; sigma_s = ss;
        sigma_a = sigma_anf + sigma_af;
        sigma_t = sigma_a + sigma_s;
        PreProcess();
        ValidateData();
    }
     ~FluorescentScatteringGridDensity() { delete[] fluorescenceDensity; }

    // Pre-processing
    void PreProcess();
    void ValidateData() const;

    // Geometry
    BBox WorldBound() const { return Inverse(WorldToVolume)(extent); }
    bool IntersectP(const Ray &r, float *t0, float *t1) const {
        Ray ray = WorldToVolume(r);
        return extent.IntersectP(ray, t0, t1);
    }
    bool SkipEmptySpace(const Ray &ray, float *tSkip) const {
        Warning("Unimplemented method");
        return false;
    }

    // Data
    float Density(const Point &p) const;
    float PhotonDensity(const Point &p) const { return 0.f; }
    float F(int x, int y, int z) const {
        x = Clamp(x, 0, nx-1);
        y = Clamp(y, 0, ny-1);
        z = Clamp(z, 0, nz-1);
        return fluorescenceDensity[z*nx*ny + y*nx + x];
    }

    // Optical Properties
    // Spectral Optical Properties
    Spectrum Sigma_a(const Point &p, const Vector &, float) const {
        const Point Pobj = WorldToVolume(p);
        return sigma_a * Density(Pobj);
    }
    Spectrum Sigma_af(const Point &p, const Vector &, float) const {
        const Point Pobj = WorldToVolume(p);
        return sigma_af * Density(Pobj);
    }
    Spectrum Sigma_s(const Point &p, const Vector &, float) const {
        const Point Pobj = WorldToVolume(p);
        return sigma_s * Density(Pobj);
    }
    Spectrum Sigma_t(const Point &p, const Vector &, float) const {
        const Point Pobj = WorldToVolume(p);
        return sigma_t * Density(Pobj);
    }
    Spectrum MaxSigma_t() const {
        return 1.f; // density * s_scale * (sigma_t);
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
        return Distance(ray(t0), ray(t1)) * (sigma_t * 1.0);
    }

    // Optical Properties at Specific Wavelength _wl_
    float Sigma_a(const Point &p, const Vector &, float, const int &wl) const {
        const Point Pobj = WorldToVolume(p);
        return sigma_a.Power(wl) * Density(Pobj);
    }
    float Sigma_af(const Point &p, const Vector &, float, const int &wl) const {
        return  0.f;
    }
    float Sigma_s(const Point &p, const Vector &, float, const int &wl) const {
        const Point Pobj = WorldToVolume(p);
        return sigma_s.Power(wl) * Density(Pobj);
    }
    float Sigma_t(const Point &p, const Vector &, float, const int &wl) const {
        const Point Pobj = WorldToVolume(p);
        return sigma_t.Power(wl) * Density(Pobj);
    }
    float MaxSigma_t(const int &wl) const {
        return 1.0 *
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
        const float sigma_t =
                (sigma_a.Power(wl) + sigma_af.Power(wl) + sigma_s.Power(wl));
        return Distance(ray(t0), ray(t1)) * (sigma_t);
    }

    // Fluorescence
    bool IsFluorescent() const { return true; }
    float Fluorescence(const Point &Pobj) const {
        return Density(Pobj);
    }
    Spectrum fEx(const Point &p) const {
        const Point Pobj = WorldToVolume(p);
        return f_ex * Density(Pobj);
    }
    Spectrum fEm(const Point &p) const {
        const Point Pobj = WorldToVolume(p);
        return f_em * Density(Pobj);
    }
    float Yeild(const Point &p) const {
        const Point Pobj = WorldToVolume(p);
        return yield * Density(Pobj);
    }
    float pf(const Point &p, const Vector &wi, const Vector &wo, float) const {
        if (!extent.Inside(WorldToVolume(p))) return 0.;
        return PhaseHG(wi, wo, gf);
    }

    // Fluorescence at Specific Wavelength _wl_
    float fEx(const Point &p, const int &wl) const {
        const Point Pobj = WorldToVolume(p);
        return f_ex.Power(wl) * Density(Pobj);
    }
    float fEm(const Point &p, const int &wl) const {
        const Point Pobj = WorldToVolume(p);
        return f_em.Power(wl) * Density(Pobj);
    }

    // Medium Sampling
    bool SampleDistance(const Ray& ray, float* tDis, Point &Psample,
        float* pdf, RNG &rng) const;
    bool SampleDirection(const Point &p, const Vector& wi, Vector& wo,
        float* pdf, RNG &rng) const;

    // Medium Sampling at Specific Wavelength _wl_
    bool SampleDistance(const Ray& ray, float* tDis, Point &Psample,
        float* pdf, RNG &rng, const int &wl) const;

private:
    // FluorescentScatteringGridDensity Private Data
    Spectrum sigma_anf, sigma_af, f_ex, f_em;
    float epsilon, c, yield, gf;
    float *fluorescenceDensity;
    float maxDensity, minDensity;
    const int nx, ny, nz;
    const BBox extent;
};


FluorescentScatteringGridDensity *CreateFluorescentGrid(const Transform &volume2world,
        const ParamSet &params);

}
#endif // PBRT_VOLUMES_FLUORESCENTSCATTERINGVOLUMEGRID_H
