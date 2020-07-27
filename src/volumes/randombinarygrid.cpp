
/*
    pbrt source code Copyright(c) 1998-2010 Matt Pharr and Greg Humphreys.
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


// volumes/randombinaryvolumegrid.cpp*
#include "stdafx.h"
#include "volumes/randombinarygrid.h"
#include "binarygrid.h"
#include "grid.h"
#include "montecarlo.h"
#include "volumeutil.h"
#include "paramset.h"
#include <limits>
#include <fstream>
#include <math.h>

float SetRandomVoxelValue(float LO = 1, float HI = 255) {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    return LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));

}

// RandomBinaryVolumeGridDensity Method Definitions
float RandomBinaryVolumeGrid::Density(const Point &Pobj) const {
    if (!extent.Inside(Pobj)) return 0;
    // Compute voxel coordinates and offsets for _Pobj_
    Vector vox = extent.Offset(Pobj);
    vox.x = vox.x * nx - .5f;
    vox.y = vox.y * ny - .5f;
    vox.z = vox.z * nz - .5f;
    int vx = Floor2Int(vox.x), vy = Floor2Int(vox.y), vz = Floor2Int(vox.z);
    float dx = vox.x - vx, dy = vox.y - vy, dz = vox.z - vz;

    // Trilinearly interpolate density values to compute local density
    float d00 = Lerp(dx, D(vx, vy, vz),     D(vx+1, vy, vz));
    float d10 = Lerp(dx, D(vx, vy+1, vz),   D(vx+1, vy+1, vz));
    float d01 = Lerp(dx, D(vx, vy, vz+1),   D(vx+1, vy, vz+1));
    float d11 = Lerp(dx, D(vx, vy+1, vz+1), D(vx+1, vy+1, vz+1));
    float d0 = Lerp(dy, d00, d10);
    float d1 = Lerp(dy, d01, d11);
    return Lerp(dz, d0, d1);
}


float RandomBinaryVolumeGrid::D(uint64 x, uint64 y, uint64 z) const {
    x = Clamp(x, 0, nx-1);
    y = Clamp(y, 0, ny-1);
    z = Clamp(z, 0, nz-1);
    uint64 bit = z*nx*ny + y*nx + x;
    return ((density->GetBit(bit)) ? densityScale * SetRandomVoxelValue(): 0.f);
}

bool RandomBinaryVolumeGrid::SampleDistance(const Ray& ray, float* tDis,
                                       Point& Psample, float* pdf,
                                       RNG &rng) const {

    // This function implements the unbiased distance sampling using woodcock
    // tracking method.
    // The idea about this algorithm is to take many steps as if the medium is
    // denser than it is, and then randomly choose to return that distance or
    // take another step.

    // Find where the ray intersects with the bounding box to evaluate
    // tMin and tMax correctly.
    BBox volumeBoundary = WorldBound();
    float tHit0, tHit1;
    if(!volumeBoundary.IntersectP(ray, &tHit0, &tHit1))
        return false; // If the ray doesn't intersect the volume, ERROR
    const float tMin = std::max(tHit0, ray.mint);
    const float tMax = std::min(tHit1, ray.maxt);

    //printf("%f %f %f %f %f %f \n", tHit0, tHit1, ray.mint, ray.maxt, tMin, tMax);
    Point p0 = ray(tMin);
    Point p1 = ray(tMax);

    // Prepare for volumyoute integration stepping
    // TODO: Change the step value
    int nSamples = Ceil2Int((tMax-tMin) / 0.5);
    float step = (tMax - tMin) / nSamples;
    float t = 0.f;
    Point p = ray(tMin);
    Spectrum mu_tCurrent(0.f);
    Spectrum mu_tIntegrated(0.f);

    while (true) {
        if(t >= tMax)
            break;
        t -= log10(1 - rng.RandomFloat()) / MaxSigma_t().y();
        if (rng.RandomFloat() < (Sigma_t(p, ray.d, 0).y() / MaxSigma_t().y()))
            break;
        p = ray(t);
        mu_tCurrent = Sigma_t(p, -ray.d, 0);
        mu_tIntegrated += mu_tCurrent;

        Spectrum T = Exp(-mu_tIntegrated * step);
        *pdf = mu_tCurrent.y() * T.y();
    }
    Psample = p;
    *tDis = t;
    return true;
}


bool RandomBinaryVolumeGrid::SampleDistance(const Ray &ray, float *tDis,
        Point &Psample, float *pdf, RNG &rng, const int &wl) const {
    return false;
}

bool RandomBinaryVolumeGrid::SampleDirection(const Point &p, const Vector& wi,
                                        Vector& wo, float* pdf, RNG &rng) const {
    wo = SampleHG(wi, g, rng.RandomFloat(), rng.RandomFloat());
    *pdf = PhaseHG(wi, wo, g);
    return true;
}


RandomBinaryVolumeGrid *CreateRandomBinaryGridVolumeRegion(const Transform &volume2world,
        const ParamSet &params) {
    // Initialize common volume region parameters
    Spectrum sigma_a = params.FindOneSpectrum("sigma_a", 0.);
    Spectrum sigma_s = params.FindOneSpectrum("sigma_s", 0.);
    float g = params.FindOneFloat("g", 0.);
    Spectrum Le = params.FindOneSpectrum("Le", 0.);
    Point p0 = params.FindOnePoint("p0", Point(0,0,0));
    Point p1 = params.FindOnePoint("p1", Point(1,1,1));
    float density = params.FindOneFloat("density", 1.);
    std::string prefix = params.FindOneString("prefix", "");
    uint64 nx = 0, ny = 0, nz = 0;
    BitArray *data = ReadBinaryVolume(prefix, nx, ny, nz);
    return new RandomBinaryVolumeGrid(sigma_a, sigma_s, g, Le, BBox(p0, p1),
                    volume2world, nx, ny, nz, data, density);
}

