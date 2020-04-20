
/*
    pbrt source code Copyright(c) 2012-2016 Marwan Abdellah.

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

#ifndef PBRT_VOLUMES_ABSORBING_H
#define PBRT_VOLUMES_ABSORBING_H

// volumes/absorbing.h*
#include "volume.h"
#include "montecarlo.h"

namespace pbrt
{

// AbsorbingVolume Declarations
class AbsorbingVolume : public VolumeRegion {
public:

    // AbsorbingVolume Public Methods
    AbsorbingVolume(const BBox &e, const Transform &v2w, float eps = 0.f,
            float conc = 0.f,float gg = 0.f) {
        WorldToVolume = Inverse(v2w);
        g = gg;
        extent = e;
        c = conc;
        epsilon = eps;
        mu = (LN10 * c * epsilon); // ln(10) * C [M] * epsilon[/cm /M]
        ValidateData();
    }
    void ValidateData() const;
    BBox WorldBound() const { return Inverse(WorldToVolume)(extent); }
    bool IntersectP(const Ray &r, float *t0, float *t1) const {
        Ray ray = WorldToVolume(r);
        return extent.IntersectP(ray, t0, t1);
    }
    Spectrum Mu(const Point &p, const Vector &, float) const {
        return extent.Inside(WorldToVolume(p)) ? mu : 0.;
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
    float p(const Point &p, const Vector &wi, const Vector &wo, float) const {
        if (!extent.Inside(WorldToVolume(p))) return 0.;
        return PhaseHG(wi, wo, g);
    }
    Spectrum tau(const Ray &ray, float, float) const {
        float t0, t1;
        if (!IntersectP(ray, &t0, &t1)) return 0.;
        return Distance(ray(t0), ray(t1)) * mu;
    }
    float Mu(const Point &p, const Vector &, float, const int &wl) const {
        return extent.Inside(WorldToVolume(p)) ? mu.Power(wl) : 0.;
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
private:
    // AbsorbingVolume Private Data
    Spectrum mu;
    float epsilon, c, g;
    BBox extent;
    Transform WorldToVolume;
};


AbsorbingVolume *CreateAbsorbingVolumeRegion
(const Transform &volume2world, const ParamSet &params);

}

#endif // PBRT_VOLUMES_ABSORBING_H
