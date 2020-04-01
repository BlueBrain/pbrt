
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

#ifndef PBRT_VOLUMES_ANNOTATEDGRID_H
#define PBRT_VOLUMES_ANNOTATEDGRID_H

// volumes/annotatedgrid.h*
#include "volume.h"

namespace pbrt
{

// AnnotatedVolumeGrid Declarations
class AnnotatedVolumeGrid : public DensityRegion {
public:
    // AnnotatedVolumeGrid Public Methods
    AnnotatedVolumeGrid(const Spectrum *sa, const Spectrum *ss, float gg,
            const Spectrum *em, const BBox &e, const Transform &v2w,
            uint64 x, uint64 y, uint64 z, float *d, uint8 *ind, const int &ntags)
        : DensityRegion(0.f, 0.f, 0.f, 0.f, v2w), sig_a(sa), sig_s(ss), le(em),
            nx(x), ny(y), nz(z), extent(e), nTags(ntags) {
        indices = ind; density = d;
        PreProcess();
    }
     ~AnnotatedVolumeGrid() { delete[] indices; }
    BBox WorldBound() const { return Inverse(WorldToVolume)(extent); }
    bool IntersectP(const Ray &r, float *t0, float *t1) const {
        Ray ray = WorldToVolume(r);
        return extent.IntersectP(ray, t0, t1);
    }
    uint64 Index(const Point &Pobj) const;
    float Density(const Point &Pobj) const;
    float D(int x, int y, int z) const {
        x = Clamp(x, 0, nx-1);
        y = Clamp(y, 0, ny-1);
        z = Clamp(z, 0, nz-1);
        return indices[x + nx*y + nx*ny*z];
    }
    Spectrum Sigma_a(const Point &p, const Vector &, float) const {
        Point Pobj = WorldToVolume(p);
        int tagIndex = Index(Pobj);
        if(tagIndex > 0)
            return Density(Pobj) * sig_a[tagIndex-1];
        else return 0;
    }
    Spectrum Sigma_s(const Point &p, const Vector &, float) const {
        Point Pobj = WorldToVolume(p);
        int tagIndex = Index(Pobj);
        if(tagIndex > 0)
            return Density(Pobj) * sig_s[tagIndex-1];
        else return 0;
    }
    Spectrum Sigma_t(const Point &p, const Vector &, float) const {
        Point Pobj = WorldToVolume(p);
        int tagIndex = Index(Pobj);
        if(tagIndex > 0)
            return Density(Pobj) * (sig_a[tagIndex-1] + sig_s[tagIndex-1]);
        else return 0;
    }
private:
    // AnnotatedVolumeGrid Private Data
    const Spectrum *sig_a;
    const Spectrum *sig_s;
    const Spectrum *le;
    float *density;
    uint8 *indices;
    float manDensity, minDensity;
    uint64 nx, ny, nz;
    const BBox extent;
    const int nTags;
};


AnnotatedVolumeGrid *CreateAnnotatedVolumeGrid(const Transform &volume2world,
        const ParamSet &params);

}
#endif // PBRT_VOLUMES_ANNOTATEDGRID_H
