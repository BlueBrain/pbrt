
/*
    pbrt source code Copyright(c) 1998-2010 Matt Pharr and Greg Humphreys.
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


// volumes/grid.cpp*
#include "stdafx.h"
#include "grid.h"
#include "volumeutil.h"
#include "paramset.h"
#include <limits>
#include <fstream>
#include <math.h>
#include "montecarlo.h"

namespace pbrt
{


// VolumeGridDensity Method Definitions
float VolumeGrid::Density(const Point &Pobj) const {
    if (!extent.Inside(Pobj)) return 0;
    // Compute voxel coordinates and offsets for _Pobj_
    Vector vox = extent.Offset(Pobj);
    vox.x = vox.x * nx - .5f;
    vox.y = vox.y * ny - .5f;
    vox.z = vox.z * nz - .5f;
    uint64 vx = Floor2Int(vox.x), vy = Floor2Int(vox.y), vz = Floor2Int(vox.z);
    float dx = vox.x - vx, dy = vox.y - vy, dz = vox.z - vz;

    // Trilinearly interpolate density values to compute local density
    float d00 = ::Lerp(dx, D(vx, vy, vz),     D(vx+1, vy, vz));
    float d10 = ::Lerp(dx, D(vx, vy+1, vz),   D(vx+1, vy+1, vz));
    float d01 = ::Lerp(dx, D(vx, vy, vz+1),   D(vx+1, vy, vz+1));
    float d11 = ::Lerp(dx, D(vx, vy+1, vz+1), D(vx+1, vy+1, vz+1));
    float d0 = ::Lerp(dy, d00, d10);
    float d1 = ::Lerp(dy, d01, d11);
    return ::Lerp(dz, d0, d1);
}


void VolumeGrid::PreProcess() {
    // Compute the minimum and maximum optical properties
    float maxVolumeDensity = std::numeric_limits<float>::min();
    float minVolumeDensity = std::numeric_limits<float>::max();

    size_t index = 0;
    for(uint64 i = 0; i < nx; i++) {
        for(uint64 j = 0; j < ny; j++) {
            for(uint64 k = 0; k < nz; k++) {
                if(density[index] > maxVolumeDensity)
                    maxVolumeDensity = density[index];
                if(density[index] < minVolumeDensity)
                    minVolumeDensity = density[index];
                index++;
            }
        }
    }
    maxDensity = maxVolumeDensity;
    minDensity = minVolumeDensity;
}


VolumeGrid *CreateGridVolumeRegion(const Transform &volume2world,
        const ParamSet &params) {
    // Initialize common volume region parameters
    Spectrum sigma_a = params.FindOneSpectrum("sigma_a", 0.);
    Spectrum sigma_s = params.FindOneSpectrum("sigma_s", 0.);
    float g = params.FindOneFloat("g", 0.);
    Spectrum Le = params.FindOneSpectrum("Le", 0.);
    Point p0 = params.FindOnePoint("p0", Point(0,0,0));
    Point p1 = params.FindOnePoint("p1", Point(1,1,1));
    std::string format = params.FindOneString("format", "pbrt");
    float* data;
    VolumeGrid* volume;
    if (format == std::string("raw")) {
        std::string prefix = params.FindOneString("prefix", "");
        Info("Reading a RAW volume from %s \n", prefix.c_str());
        u_int64_t nx, ny, nz;
        data = ReadFloatVolume(prefix, nx, ny, nz);
        volume = new VolumeGrid(sigma_a, sigma_s, g, Le, BBox(p0, p1),
                        volume2world, nx, ny, nz, data);

    } else if (format == std::string("pbrt")) {
        Info("Reading a PBRT volume file with density \n");
        int nitems;
        const float *data = params.FindFloat("density", &nitems);
        if (!data) {
            Error("No \"density\" values provided for volume grid?");
            return NULL;
        }
        uint64 nx = params.FindOneInt("nx", 1);
        uint64 ny = params.FindOneInt("ny", 1);
        uint64 nz = params.FindOneInt("nz", 1);
        if (nitems != nx*ny*nz) {
            Error("VolumeGridDensity has %d density values but nx*ny*nz = %d",
                  nitems, nx*ny*nz);
            return NULL;
        }
        volume = new VolumeGrid(sigma_a, sigma_s, g, Le, BBox(p0, p1),
                        volume2world, nx, ny, nz, data);
    }
    return volume;
}


bool VolumeGrid::SampleDistance(const Ray& ray, float* tDis,
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

   // printf("%f %f %f \n", p0.x, p0.y, p0.z);
   // printf("%f %f %f \n", p1.x, p1.y, p1.z);



    // Prepare for volumyoute integration stepping
    // TODO: Change the step value
    uint64 nSamples = Ceil2Int((tMax-tMin) / 0.5);
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

bool VolumeGrid::SampleDistance(const Ray& ray, float* tDis,
        Point &Psample, float* pdf, RNG &rng, const uint64 &wl) const {
    return false;
}


bool VolumeGrid::SampleDirection(const Point &p, const Vector& wi,
                                        Vector& wo, float* pdf, RNG &rng) const {
    wo = SampleHG(wi, g, rng.RandomFloat(), rng.RandomFloat());
    *pdf = PhaseHG(wi, wo, g);
    return true;
}

}
