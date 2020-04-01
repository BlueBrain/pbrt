
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

#ifndef PBRT_VOLUMES_FLUORESCENT_H
#define PBRT_VOLUMES_FLUORESCENT_H

// volumes/fluorescent.h*
#include "volume.h"
#include "montecarlo.h"

namespace pbrt
{

// FluorescentVolumeDensity Declarations
class FluorescentVolumeDensity : public VolumeRegion {
public:

    // FluorescentVolumeDensity Public Methods
    FluorescentVolumeDensity(const BBox &e, const Transform &v2w,
            const Spectrum &fex = 0.f, const Spectrum &fem = 0.f,
            float eps = 0.f, float conc = 0.f, float phi = 0.f,
            float ggf = 0.f) {
        WorldToVolume = Inverse(v2w);
        yield = phi; gf = ggf; extent = e;
        f_ex = fex; f_em = fem;
        f_ex.Normalize(); f_em.Normalize(); f_em.NormalizeSPDArea();
        epsilon = eps; c = conc;
        mu = (LN10 * c * epsilon);
        ValidateData();
    }
    void ValidateData() const;
    BBox WorldBound() const {
        return Inverse(WorldToVolume)(extent);
    }
    bool IntersectP(const Ray &r, float *t0, float *t1) const {
        Ray ray = WorldToVolume(r);
        return extent.IntersectP(ray, t0, t1);
    }
    bool HasNonClearedFluorescentVolumes() const {
        return false;
    }
    Spectrum Mu(const Point &p, const Vector &, float) const {
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ? (mu) : 0.;
    }
    Spectrum Sigma_a(const Point &p, const Vector &, float) const {
        return Mu(p, Vector(), 0.f);
    }
    Spectrum Sigma_af(const Point &p, const Vector &, float) const {
        return Mu(p, Vector(), 0.f);
    }
    Spectrum Sigma_s(const Point &p, const Vector &, float) const {
        return Mu(p, Vector(), 0.f);
    }
    Spectrum Sigma_t(const Point &p, const Vector &, float) const {
        return Mu(p, Vector(), 0.f);
    }
    Spectrum tau(const Ray &ray, float, float) const {
        float t0, t1;
        if (!IntersectP(ray, &t0, &t1)) return 0.;
        return Distance(ray(t0), ray(t1)) * (mu);
    }
    float Mu(const Point &p, const Vector &, float, const int &wl) const {
        const Point Pobj = WorldToVolume(p);
        return extent.Inside(Pobj) ? mu.Power(wl) : 0.;
    }
    float Sigma_a(const Point &p, const Vector &, float, const int &wl) const {
        return Mu(p, Vector(), 0.f, wl);
    }
    float Sigma_af(const Point &p, const Vector &, float, const int &wl) const {
        return Mu(p, Vector(), 0.f, wl);
    }
    float Sigma_s(const Point &p, const Vector &, float, const int &wl) const {
        return Mu(p, Vector(), 0.f, wl);
    }
    float Sigma_t(const Point &p, const Vector &, float, const int &wl) const {
        return Mu(p, Vector(), 0.f, wl);
    }
    float tauLambda(const Ray &ray, float, float, const int &wl) const {
        float t0, t1;
        if (!IntersectP(ray, &t0, &t1)) return 0.;
        return Distance(ray(t0), ray(t1)) * mu.Power(wl);
    }
    bool IsFluorescent() const {
        return true;
    }
    float Fluorescence(const Point &Pobj) const {
        if (extent.Inside(Pobj)) return 1.0;
        return 0.f;
    }
    Spectrum fEx(const Point &p) const {
        return f_ex;
    }
    Spectrum fEm(const Point &p) const {
        return f_em;
    }
    float Yeild(const Point &p) const {
        return yield;
    }
    float pf(const Point &p, const Vector &wi, const Vector &wo, float) const {
        if (!extent.Inside(WorldToVolume(p))) return 0.;
        return PhaseHG(wi, wo, gf);
    }
    float fEx(const Point &p, const int &wl) const {
        return f_ex.Power(wl);
    }
    float fEm(const Point &p, const int &wl) const {
        return f_em.Power(wl);
    }
private:
    // FluorescentVolumeDensity Private Data
    Spectrum mu, f_ex, f_em;
    float epsilon, c, yield, gf;
    BBox extent;
    Transform WorldToVolume;
};


FluorescentVolumeDensity *CreateFluorescentVolumeDensityRegion
(const Transform &volume2world, const ParamSet &params);

}
#endif // PBRT_VOLUMES_FLUORESCENT_H
