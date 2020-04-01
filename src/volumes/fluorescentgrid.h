
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

#ifndef PBRT_VOLUMES_FLUORESCENTVOLUMEGRID_H
#define PBRT_VOLUMES_FLUORESCENTVOLUMEGRID_H

// volumes/fluorescentgrid.h*
#include "volume.h"

namespace pbrt
{

// FluorescentGridDensity Declarations
class FluorescentGridDensity : public DensityRegion {
public:
    // FluorescentGridDensity Public Methods
    FluorescentGridDensity(const BBox &e, const Transform &v2w,
            int x, int y, int z, const uchar *data,
            const Spectrum &fex, const Spectrum &fem,
            float eps, float conc, float phi, float ggf)
        : DensityRegion(0.f, 0.f, 0.f, 0.f, v2w), nx(x), ny(y), nz(z),
            extent(e), yield(phi), gf(ggf), grid(data) {
        f_ex = fex; f_em = fem;
        f_ex.Normalize(); f_em.Normalize(); f_em.NormalizeSPDArea();
        epsilon = eps; c = conc;
        mu = (LN10 * c * epsilon * Spectrum(1.f));
        ValidateData();
    }
     ~FluorescentGridDensity() { delete[] grid; }
    void ValidateData();
    BBox WorldBound() const { return Inverse(WorldToVolume)(extent); }
    bool IntersectP(const Ray &r, float *t0, float *t1) const {
        Ray ray = WorldToVolume(r);
        return extent.IntersectP(ray, t0, t1);
    }
    bool HasNonClearedFluorescentVolumes() const {
        return false;
    }
    bool IsFluorescent() const {
        return true;
    }
    float Fluorescence(const Point &p) const;
    virtual float F(int x, int y, int z) const;
    Spectrum Mu(const Point &p, const Vector &, float) const {
        if(Fluorescence(WorldToVolume(p)))
            return mu;
        else
            return 0.f;
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
    Spectrum STER(const Point &p, const Vector &w, float time) const;
    Spectrum ATER(const Point &p, const Vector &w, float time) const;
    Spectrum tau(const Ray &ray, float, float) const {
        float t0, t1;
        if (!IntersectP(ray, &t0, &t1)) return 0.;
        return Distance(ray(t0), ray(t1)) * (mu);
    }
    float Mu(const Point &p, const Vector &, float, const int &wl) const {
        if(Fluorescence(WorldToVolume(p)))
            return mu.Power(wl);
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
    float STER(const Point &p, const Vector &w, float time, const int &wl) const;
    float ATER(const Point &p, const Vector &w, float time, const int &wl) const;
    float tauLambda(const Ray &ray, float, float, const int &wl) const {
        float t0, t1;
        if (!IntersectP(ray, &t0, &t1)) return 0.;
        return Distance(ray(t0), ray(t1)) * mu.Power(wl);
    }
    Spectrum fEx(const Point &p) const {
        return f_ex;
        if(Fluorescence(WorldToVolume(p)) > 0)
            return f_ex;
        return 0.f;
    }
    Spectrum fEm(const Point &p) const {
        return f_em;
        if(Fluorescence(WorldToVolume(p)) > 0)
            return f_em;
        return 0.f;
    }
    float Yeild(const Point &p) const {
        return yield;
        if(Fluorescence(WorldToVolume(p)) > 0)
            return yield;
        return 0.f;
    }
    float pf(const Point &p, const Vector &wi, const Vector &wo, float) const {
        if (Fluorescence(WorldToVolume(p)) > 0)
            return PhaseHG(wi, wo, gf);
        return 0.f;
    }
    float fEx(const Point &p, const int &wl) const {
        if(Fluorescence(WorldToVolume(p)) > 0)
            return f_ex.Power(wl);
        return 0.f;
    }
    float fEm(const Point &p, const int &wl) const {
        if(Fluorescence(WorldToVolume(p)) > 0)
            return f_em.Power(wl);
        return 0;
    }
protected:
    // FluorescentGridDensity Protected Data
    Spectrum mu, f_ex, f_em;
    float epsilon, c, yield, gf;
    const int nx, ny, nz;
    const BBox extent;
private:
    // FluorescentGridDensity Protected Data
    const uchar *grid;
};


FluorescentGridDensity *CreateFluorescentGrid(const Transform &volume2world,
        const ParamSet &params);

}

#endif // PBRT_VOLUMES_FLUORESCENTVOLUMEGRID_H
