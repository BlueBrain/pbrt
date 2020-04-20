
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


// volumes/fluorescentscatteringgrid.cpp*
#include "stdafx.h"
#include "fluorescentscatteringgrid.h"
#include "volumeutil.h"
#include "paramset.h"
#include <limits>
#include <fstream>
#include <math.h>
#include "montecarlo.h"

namespace pbrt
{

// FluorescentScatteringGridDensity Method Definitions
float FluorescentScatteringGridDensity::Density(const Point &Pobj) const {
    if (!extent.Inside(Pobj)) return 0;
    // Compute voxel coordinates and offsets for _Pobj_
    Vector vox = extent.Offset(Pobj);
    vox.x = vox.x * nx - .5f;
    vox.y = vox.y * ny - .5f;
    vox.z = vox.z * nz - .5f;
    int vx = Floor2Int(vox.x), vy = Floor2Int(vox.y), vz = Floor2Int(vox.z);
    float dx = vox.x - vx, dy = vox.y - vy, dz = vox.z - vz;

    // Trilinearly interpolate density values to compute local density
    float f00 = ::Lerp(dx, F(vx, vy, vz),     F(vx+1, vy, vz));
    float f10 = ::Lerp(dx, F(vx, vy+1, vz),   F(vx+1, vy+1, vz));
    float f01 = ::Lerp(dx, F(vx, vy, vz+1),   F(vx+1, vy, vz+1));
    float f11 = ::Lerp(dx, F(vx, vy+1, vz+1), F(vx+1, vy+1, vz+1));
    float f0 = ::Lerp(dy, f00, f10);
    float f1 = ::Lerp(dy, f01, f11);
    return ::Lerp(dz, f0, f1);
}


void FluorescentScatteringGridDensity::PreProcess() {
    // Compute the minimum and maximum optical properties
    float maxVolumeDensity = std::numeric_limits<float>::min();
    float minVolumeDensity = std::numeric_limits<float>::max();

    size_t index = 0;
    for(int i = 0; i < nx; i++) {
        for(int j = 0; j < ny; j++) {
            for(int k = 0; k < nz; k++) {
                if(fluorescenceDensity[index] > maxVolumeDensity)
                    maxVolumeDensity = fluorescenceDensity[index];
                if(fluorescenceDensity[index] < minVolumeDensity)
                    minVolumeDensity = fluorescenceDensity[index];
                index++;
            }
        }
    }
    maxDensity = maxVolumeDensity;
    maxDensity = minVolumeDensity;
}


void FluorescentScatteringGridDensity::ValidateData() const {
    f_ex.WriteSPD("fex");
    f_ex.WriteSPD("fem");
    sigma_af.WriteSPD("sigma_af");
    sigma_anf.WriteSPD("sigma_anf");
    sigma_a.WriteSPD("sigma_a");
    sigma_s.WriteSPD("sigma_s");
    sigma_t.WriteSPD("sigma_t");
}


FluorescentScatteringGridDensity *CreateFluorescentGrid(const Transform &volume2world,
        const ParamSet &params) {
    // Initialize common volume region parameters
    Spectrum sigma_a = params.FindOneSpectrum("sigma_a", 0.);
    Spectrum sigma_s = params.FindOneSpectrum("sigma_s", 0.);
    Spectrum fex = params.FindOneSpectrum("fex", 0.);
    Spectrum fem = params.FindOneSpectrum("fem", 0.);
    float epsilon = params.FindOneFloat("epsilon", 0.);
    float c = params.FindOneFloat("c", 0.);
    float yield = params.FindOneFloat("yield", 1.);
    float g = params.FindOneFloat("g", 0.);
    float gf = params.FindOneFloat("gf", 0.);
    Spectrum Le = params.FindOneSpectrum("Le", 0.);
    Point p0 = params.FindOnePoint("p0", Point(0,0,0));
    Point p1 = params.FindOnePoint("p1", Point(1,1,1));
    std::string format = params.FindOneString("format", "raw");
    float* data;
    if (format == std::string("raw")) {
        std::string prefix = params.FindOneString("prefix", "");
        Info("Reading a RAW volume from %s \n", prefix.c_str());
        u_int64_t nx, ny, nz;
        data = ReadFloatVolume(prefix, nx, ny, nz);
        return new FluorescentScatteringGridDensity(sigma_a, sigma_s, g, Le,
            BBox(p0, p1), volume2world, nx, ny, nz, data, fex, fem, epsilon, c,
            yield, gf);
    } else {
        Info("Reading a PBRT volume file with density \n");
        int nitems;
        const float *data = params.FindFloat("density", &nitems);
        if (!data) {
            Error("No \"density\" values provided for volume grid?");
            return NULL;
        }
        int nx = params.FindOneInt("nx", 1);
        int ny = params.FindOneInt("ny", 1);
        int nz = params.FindOneInt("nz", 1);
        if (nitems != nx*ny*nz) {
            Error("FluorescentScatteringGridDensity has %d density values but nx*ny*nz = %d",
                  nitems, nx*ny*nz);
            return NULL;
        }
        return new FluorescentScatteringGridDensity(sigma_a, sigma_s, g, Le,
            BBox(p0, p1), volume2world, nx, ny, nz, data, fex, fem, epsilon, c,
            yield, gf);
    }
}


bool FluorescentScatteringGridDensity::SampleDistance(const Ray& ray, float* tDis,
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

bool FluorescentScatteringGridDensity::SampleDistance(const Ray& ray, float* tDis,
        Point &Psample, float* pdf, RNG &rng, const int &wl) const {
    return false;
}


bool FluorescentScatteringGridDensity::SampleDirection(const Point &p, const Vector& wi,
                                        Vector& wo, float* pdf, RNG &rng) const {
    wo = SampleHG(wi, g, rng.RandomFloat(), rng.RandomFloat());
    *pdf = PhaseHG(wi, wo, g);
    return true;
}

}
