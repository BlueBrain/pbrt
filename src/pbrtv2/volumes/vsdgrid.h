
/*
    pbrt source code Copyright(c) 2012-2016 Marwan Abdellah.
    Blue Brain Project (BBP) / Ecole Polytechnique Fédérale de Lausanne (EPFL)
    Lausanne CH-1015, Switzerland.

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

#ifndef PBRT_VOLUMES_VSDVOLUMEGRID_H
#define PBRT_VOLUMES_VSDVOLUMEGRID_H

// volumes/vsdgrid.h*
#include "volume.h"
#include <limits>

using namespace std;

namespace pbrt
{

// VSDVolumeGrid Declarations
class VSDVolumeGrid : public DensityRegion {
public:
    // VSDVolumeGrid Public Methods
    VSDVolumeGrid(const Spectrum &sa, const Spectrum &ss, float gg,
            const Spectrum &em, const BBox &e, const Transform &v2w,
            uint64 x, uint64 y, uint64 z, const float *d)
        : DensityRegion(sa, ss, gg, em, v2w),
          nx(x), ny(y), nz(z), vsdSignalExtent(e) {
        density = new float[nx*ny*nz];
        memcpy(density, d, nx*ny*nz*sizeof(float));

        // Set the parameters of the semi infinite extent.
        semiInfiniteExtent.pMin.x = vsdSignalExtent.pMin.x * 1000;
        semiInfiniteExtent.pMin.y = vsdSignalExtent.pMin.y;
        semiInfiniteExtent.pMin.z = vsdSignalExtent.pMin.z * 1000;
        semiInfiniteExtent.pMax.x = vsdSignalExtent.pMax.x * 1000;
        semiInfiniteExtent.pMax.y = vsdSignalExtent.pMax.y;
        semiInfiniteExtent.pMax.z = vsdSignalExtent.pMax.z * 1000;

        PreProcess();
    }
     ~VSDVolumeGrid() { delete[] density; }
    void PreProcess();
    BBox WorldBound() const { return Inverse(WorldToVolume)(semiInfiniteExtent); }
    bool IntersectP(const Ray &r, float *t0, float *t1) const {
        Ray ray = WorldToVolume(r);
        return semiInfiniteExtent.IntersectP(ray, t0, t1);
    }
    bool SampleDistance(const Ray &ray, float *tDist,
            Point &Psample, float *pdf, RNG &rng) const;
    bool SampleDirection(const Point &p, const Vector& wi,
            Vector& wo, float* pdf, RNG &rng) const;
    float Density(const Point &Pobj) const;
    float PhotonDensity(const Point &p) const;

    float D(uint64 x, uint64 y, uint64 z) const {
        x = Clamp(x, 0, nx-1);
        y = Clamp(y, 0, ny-1);
        z = Clamp(z, 0, nz-1);
        return density[z*nx*ny + y*nx + x];
    }

private:
    // VSDVolumeGrid Private Data
    float *density;
    const uint64 nx, ny, nz;
    BBox semiInfiniteExtent, vsdSignalExtent;
};


VSDVolumeGrid *CreateVSDGridVolumeRegion(const Transform &volume2world,
        const ParamSet &params);

}
#endif // PBRT_VOLUMES_VSDVOLUMEGRID_H
