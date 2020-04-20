
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

#ifndef PBRT_VOLUMES_BINARYVOLUMEGRID_H
#define PBRT_VOLUMES_BINARYVOLUMEGRID_H

// volumes/binaryvolumegrid.h*
#include "volume.h"
#include "bitarray.h"

namespace pbrt
{
// BinaryVolumeGridDensity Declarations
class BinaryVolumeGrid : public DensityRegion {
public:
    // VolumeGridDensity Public Methods
    BinaryVolumeGrid(const Spectrum &sa, const Spectrum &ss, float gg,
            const Spectrum &em, const BBox &e, const Transform &v2w,
            uint64 x, uint64 y, uint64 z, BitArray *d, const float &scale)
        : DensityRegion(sa, ss, gg, em, v2w), nx(x), ny(y), nz(z), extent(e),
          density(d), densityScale(scale) { }
    ~BinaryVolumeGrid() { density->~BitArray(); }
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
    float Density(const Point &Pobj) const;
    float D(uint64 x, uint64 y, uint64 z) const;

    // Optical Properties
    Spectrum MaxSigma_t() const {
        return (sigma_a + sigma_s) * densityScale;
    }

    // Optical Properties at Specific Wavelength _wl_
    float MaxSigma_t(const int &wl) const {
        return (sigma_a.Power(wl) + sigma_s.Power(wl)) * densityScale; }


    // Fluroescence
    float Fluorescence(const Point &Pobj) const { return false; }
    Spectrum fEx(const Point &p) const { return 0.f; }
    Spectrum fEm(const Point &p) const { return 0.f; }
    float Yeild(const Point &p) const { return 0.f; }

    //  Medium Sampling
    bool SampleDistance(const Ray &ray, float *tDis, Point &Psample, float *pdf,
            RNG &rng) const;
    bool SampleDirection(const Point &p, const Vector &wi, Vector &wo, float *pdf,
            RNG &rng) const;

    //  Medium Sampling at Specific Wavelength _wl_
    bool SampleDistance(const Ray &ray, float *tDis, Point &Psample, float *pdf,
            RNG &rng, const int &wl) const;

protected:
    // BinaryVolumeGridDensity Private Data
    const uint64 nx, ny, nz;
    const BBox extent;
    BitArray* density;
    float densityScale;
};


BinaryVolumeGrid *CreateBinaryGridVolumeRegion(const Transform &volume2world,
        const ParamSet &params);

}
#endif // PBRT_VOLUMES_BINARYVOLUMEGRID_H
