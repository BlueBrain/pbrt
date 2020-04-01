
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


// volumes/vsdgrid.cpp*
#include "stdafx.h"
#include "vsdgrid.h"
#include "volumeutil.h"
#include "paramset.h"
#include <limits>
#include <fstream>
#include <math.h>
#include "montecarlo.h"

namespace pbrt
{
// VSDVolumeGridDensity Method Definitions
float VSDVolumeGrid::Density(const Point &Pobj) const {
    // If the point is inside the semi infinite layer, return 1.0, else 0.0
    if (semiInfiniteExtent.Inside(Pobj)) return 1.f;
    return 0.f;
}


float VSDVolumeGrid::PhotonDensity(const Point &p) const {
    const Point Pobj = WorldToVolume(p);
    if (!vsdSignalExtent.Inside(Pobj)) return 0.f;
    // Compute voxel coordinates and offsets for _Pobj_
    Vector vox = vsdSignalExtent.Offset(Pobj);
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


void VSDVolumeGrid::PreProcess() {
    printf("VSD Signal Volume BB: %f %f %f %f %f %f \n",
           vsdSignalExtent.pMin.x, vsdSignalExtent.pMin.y, vsdSignalExtent.pMin.z,
           vsdSignalExtent.pMax.x, vsdSignalExtent.pMax.y, vsdSignalExtent.pMax.z);

    printf("Cortical Volume BB: %f %f %f %f %f %f \n",
           semiInfiniteExtent.pMin.x, semiInfiniteExtent.pMin.y, semiInfiniteExtent.pMin.z,
           semiInfiniteExtent.pMax.x, semiInfiniteExtent.pMax.y, semiInfiniteExtent.pMax.z);
}


bool VSDVolumeGrid::SampleDistance(const Ray &ray, float *tDist,
        Point &Psample, float *pdf, RNG &rng) const {
    // Compute the sampling step
    Vector w = -ray.d;
    float t = -log(1 - rng.RandomFloat()) / Sigma_t(ray.o, w, ray.time).y();
    *tDist = ray.mint + t;
    Psample = ray(t);

    //printf("%f %f %f, %f %f %f \n", Psample.x, Psample.y, Psample.z,
      //    ray.o.x, ray.o.y, ray.o.z);

    if (!semiInfiniteExtent.Inside(Psample)) {
        return false;
    } else {
        // Compute the PDF that is associated with this sample
        Spectrum extenctionCoeff = Sigma_t(Psample, w, ray.time);
        Spectrum samplePdf = extenctionCoeff * Exp(-extenctionCoeff * t);
        *pdf = samplePdf.y();
        return true;
    }
}


bool VSDVolumeGrid::SampleDirection(const Point &p, const Vector& wi,
        Vector& wo, float* pdf, RNG &rng) const {
    const Point Pobj = WorldToVolume(p);
    if(semiInfiniteExtent.Inside(Pobj)) {
        wo = SampleHG(wi, g, rng.RandomFloat(), rng.RandomFloat());
        *pdf = PhaseHG(wi, wo, g);
        return true;
    } else {
        return false;
    }
}


VSDVolumeGrid *CreateVSDGridVolumeRegion(const Transform &volume2world,
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
    if (format == std::string("raw")) {
        std::string prefix = params.FindOneString("prefix", "");
        Info("Reading a RAW volume from %s \n", prefix.c_str());
        uint64 nx, ny, nz;
        float maxValue;
        data = ReadVSDVolume(prefix, nx, ny, nz, p0.x, p0.y, p0.z,
                             p1.x, p1.y, p1.z, maxValue);
        return new VSDVolumeGrid(sigma_a, sigma_s, g, Le, BBox(p0, p1),
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
            Error("VSDVolumeGridDensity has %d density values but nx*ny*nz = %d",
                  nitems, nx*ny*nz);
            return NULL;
        }
        return new VSDVolumeGrid(sigma_a, sigma_s, g, Le, BBox(p0, p1),
                                     volume2world, nx, ny, nz, data);
    }
    return NULL;
}

}
