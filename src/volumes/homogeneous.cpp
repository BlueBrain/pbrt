
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


// volumes/homogeneous.cpp*
#include "stdafx.h"
#include "volumes/homogeneous.h"
#include "homogeneous.h"
#include "paramset.h"
#include "montecarlo.h"

namespace pbrt
{
// HomogeneousVolumeDensity Method Definitions
bool HomogeneousVolumeDensity::SampleDistance(const Ray &ray, float *tDist,
        Point &Psample, float *pdf, RNG &rng) const {
    // Compute the sampling step
    Vector w = -ray.d;
    float t = -log(1 - rng.RandomFloat()) / Sigma_t(ray.o, w, ray.time).y();
    *tDist = ray.mint + t;
    Psample = ray(t);

    if (!extent.Inside(Psample)) {
        return false;
    } else {
        // Compute the PDF that is associated with this sample
        Spectrum extenctionCoeff = Sigma_t(Psample, w, ray.time);
        Spectrum samplePdf = extenctionCoeff * Exp(-extenctionCoeff * t);
        *pdf = samplePdf.y();
        return true;
    }
}


bool HomogeneousVolumeDensity::SampleDistance(const Ray &ray, float *tDist,
        Point &Psample, float *pdf, RNG &rng, const int &wl) const {
    // Compute the sampling step
    Vector w = -ray.d;
    float t = -log(1 - rng.RandomFloat()) / Sigma_t(ray.o, w, ray.time, wl);
    *tDist = ray.mint + t;
    Psample = ray(t);

    if (!extent.Inside(Psample)) {
        return false;
    } else {
        // Compute the PDF that is associated with this sample
        float extenctionCoeff = Sigma_t(Psample, w, ray.time, wl);
        float samplePdf = extenctionCoeff * exp(-extenctionCoeff * t);
        *pdf = samplePdf;
        return true;
    }
}


bool HomogeneousVolumeDensity::SampleDirection(const Point &p, const Vector& wi,
        Vector& wo, float* pdf, RNG &rng) const {
    const Point Pobj = WorldToVolume(p);
    if(extent.Inside(Pobj)) {
        wo = SampleHG(wi, g, rng.RandomFloat(), rng.RandomFloat());
        *pdf = PhaseHG(wi, wo, g);
        return true;
    } else {
        return false;
    }
}


HomogeneousVolumeDensity
*CreateHomogeneousVolumeDensityRegion(const Transform &volume2world,
        const ParamSet &params) {
    // Initialize common volume region parameters
    Spectrum sigma_a = params.FindOneSpectrum("sigma_a", 0.);
    Spectrum sigma_s = params.FindOneSpectrum("sigma_s", 0.);
    float g = params.FindOneFloat("g", 0.);
    float density = params.FindOneFloat("density", 1.);
    Spectrum Le = params.FindOneSpectrum("Le", 0.);
    Point p0 = params.FindOnePoint("p0", Point(0,0,0));
    Point p1 = params.FindOnePoint("p1", Point(1,1,1));
    return new HomogeneousVolumeDensity(sigma_a, sigma_s,
                density, g, Le, BBox(p0, p1), volume2world);
}
}
