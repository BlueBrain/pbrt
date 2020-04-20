
/*
    pbrt source code Copyright(c) 2015-2016 Marwan Abdellah.

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

#ifndef PBRT_VOLUMES_FLUORESCENTANNOTATEDGRID_H
#define PBRT_VOLUMES_FLUORESCENTANNOTATEDGRID_H

// volumes/annotatedgrid.h*
#include "volume.h"

namespace pbrt
{

// FluorescentAnnotatedGrid Declarations
class FluorescentAnnotatedVolumeGrid : public DensityRegion {
public:
    // FluorescentAnnotatedGrid Public Methods
    FluorescentAnnotatedVolumeGrid(const BBox &e, const Transform &v2w,
            Spectrum *fex, Spectrum *fem, const float *eps,
            const float *conc, const float *phi, const float *ggf,
            uint64 x, uint64 y, uint64 z, uint8 *ind, const int &ntags)
        : DensityRegion(0.f, 0.f, 0.f, 0.f, v2w), epsilon(eps), c(conc),
            yield(phi), nx(x), ny(y), nz(z), extent(e), nTags(ntags) {
        indices = ind;
        yield = phi; gf = ggf;
        epsilon = eps; c = conc;
        f_ex = fex; f_em = fem;
        for (uint64 i = 0; i < nTags; ++i) {
            f_ex[i].Normalize();
            f_em[i].Normalize();
            f_em[i].NormalizeSPDArea();
        }
        // Compute _mu_ for all the fluorophores
        mu = new Spectrum[ntags];
        for (uint64 i = 0; i < nTags; ++i) {
            mu[i] = LN10 * c[i] * epsilon[i];
        }
        ValidateData();
        PreProcess();
    }
     ~FluorescentAnnotatedVolumeGrid() { delete[] indices; }
    void ValidateData() const;
    BBox WorldBound() const { return Inverse(WorldToVolume)(extent); }
    bool IntersectP(const Ray &r, float *t0, float *t1) const {
        Ray ray = WorldToVolume(r);
        return extent.IntersectP(ray, t0, t1);
    }
    bool HasNonClearedFluorescentVolumes() const {
        return false;
    }
    uint64 Index(const Point &Pobj) const;
    Spectrum Mu(const Point &p, const Vector &, float) const {
        Point Pobj = WorldToVolume(p);
        uint64 tagIndex = Index(Pobj);
        if(tagIndex > 0) {
            return  mu[tagIndex-1];
        }
        else return 0;
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
    float Mu(const Point &p, const Vector &, float, const int &wl) const {
        Mu(p, Vector(), 0.f).Power(wl);
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
    bool IsFluorescent() const {
        return true;
    }
    float Fluorescence(const Point &Pobj) const {
        int tagIndex = Index(Pobj);
        if(tagIndex > 0)
            return 1.0;
        return 0.f;
    }
    Spectrum fEx(const Point &p) const {
        Point Pobj = WorldToVolume(p);
        int tagIndex = Index(Pobj);
        if(tagIndex > 0)
            return f_ex[tagIndex-1];
        return 0;
    }
    Spectrum fEm(const Point &p) const {
        Point Pobj = WorldToVolume(p);
        int tagIndex = Index(Pobj);
        if(tagIndex > 0)
            return f_em[tagIndex-1];
        return 0;
    }
    float Yeild(const Point &p) const {
        Point Pobj = WorldToVolume(p);
        int tagIndex = Index(Pobj);
        if(tagIndex > 0)
            return yield[tagIndex-1];

        printf(".");
        return 0;
    }
    float pf(const Point &p, const Vector &wi, const Vector &wo, float) const {
        Point Pobj = WorldToVolume(p);
        int tagIndex = Index(Pobj);
        if(tagIndex > 0)
            return PhaseHG(wi, wo, gf[tagIndex-1]);
        return 0;
    }
    float fEx(const Point &p, const int &wl) const {
        Point Pobj = WorldToVolume(p);
        int tagIndex = Index(Pobj);
        if(tagIndex > 0 && tagIndex < 3)
            return f_ex[tagIndex-1].Power(wl);
        return 0;
    }
    float fEm(const Point &p, const int &wl) const {
        Point Pobj = WorldToVolume(p);
        int tagIndex = Index(Pobj);
        if(tagIndex > 0 && tagIndex < 3)
            return f_em[tagIndex-1].Power(wl);
        return 0;
    }
private:
    // FluorescentAnnotatedGrid Private Data
    Spectrum *mu, *f_ex, *f_em;
    const float *epsilon, *c, *yield, *gf;
    const uint8 *indices;
    uint64 nx, ny, nz;
    const BBox extent;
    const int nTags;
};


FluorescentAnnotatedVolumeGrid *CreateFluorescentAnnotatedVolumeGrid(const Transform &volume2world,
        const ParamSet &params);

}

#endif // PBRT_VOLUMES_FLUORESCENTANNOTATEDGRID_H
