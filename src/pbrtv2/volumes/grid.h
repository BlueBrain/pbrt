
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

#ifndef PBRT_VOLUMES_VOLUMEGRID_H
#define PBRT_VOLUMES_VOLUMEGRID_H

// volumes/grid.h*
#include "volume.h"


namespace pbrt
{
// VolumeGridDensity Declarations
class VolumeGrid : public DensityRegion {
public:
    // VolumeGridDensity Public Methods
    VolumeGrid(const Spectrum &sa, const Spectrum &ss, float gg,
            const Spectrum &em, const BBox &e, const Transform &v2w,
            uint64 x, uint64 y, uint64 z, const float *d)
        : DensityRegion(sa, ss, gg, em, v2w),
          nx(x), ny(y), nz(z), extent(e) {
        density = new float[nx*ny*nz];
        memcpy(density, d, nx*ny*nz*sizeof(float));
        PreProcess();
    }
     ~VolumeGrid() { delete[] density; }
    void PreProcess();
    BBox WorldBound() const { return Inverse(WorldToVolume)(extent); }
    bool IntersectP(const Ray &r, float *t0, float *t1) const {
        Ray ray = WorldToVolume(r);
        return extent.IntersectP(ray, t0, t1);
    }
    float Density(const Point &Pobj) const;
    float D(uint64 x, uint64 y, uint64 z) const {
        x = Clamp(x, 0, nx-1);
        y = Clamp(y, 0, ny-1);
        z = Clamp(z, 0, nz-1);
        return density[z*nx*ny + y*nx + x];
    }
    Spectrum MaxSigma_t() const { return (sigma_a + sigma_s) * maxDensity; }
    float MaxSigma_t(const uint64 &wl) const {
        return (sigma_a.Power(wl) + sigma_s.Power(wl)) * maxDensity;
    }
    bool SampleDistance(const Ray& ray, float* tDis, Point &Psample,
        float* pdf, RNG &rng) const;
    bool SampleDirection(const Point &p, const Vector& wi, Vector& wo,
        float* pdf, RNG &rng) const;
    bool SampleDistance(const Ray& ray, float* tDis, Point &Psample,
        float* pdf, RNG &rng, const uint64 &wl) const;
private:
    // VolumeGridDensity Private Data
    float *density;
    float maxDensity, minDensity;
    const uint64 nx, ny, nz;
    const BBox extent;
};


VolumeGrid *CreateGridVolumeRegion(const Transform &volume2world,
        const ParamSet &params);
}
#endif // PBRT_VOLUMES_VOLUMEGRID_H
