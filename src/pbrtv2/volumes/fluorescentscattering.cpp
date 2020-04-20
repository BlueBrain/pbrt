
/*
    pbrt source code Copyright(c) 1998-2012 Matt Pharr and Greg Humphreys.
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


// volumes/fluorescentscattering.cpp*
#include "stdafx.h"
#include "fluorescentscattering.h"
#include "paramset.h"
#include "montecarlo.h"

namespace pbrt
{

// FluorescentScatteringVolumeDensity Method Definitions
void FluorescentScatteringVolumeDensity::ValidateData() const {
    // Write the spectra to validate the input data
    f_ex.WriteSPD("fex");
    f_ex.WriteSPD("fem");
    sigma_af.WriteSPD("sigma_af");
    sigma_anf.WriteSPD("sigma_anf");
    sigma_a.WriteSPD("sigma_a");
    sigma_s.WriteSPD("sigma_s");
    sigma_t.WriteSPD("sigma_t");
}


bool FluorescentScatteringVolumeDensity::SampleDistance(const Ray &ray, float *tDist,
        Point &Psample, float *pdf, RNG &rng) const {
    float t = -log(1 - rng.RandomFloat()) / Sigma_t(ray.o, ray.d, ray.time).y();
    *tDist = ray.mint + t;
    Psample = ray(t);
    // Return false if _Psample_ is outside the volume
    if (!extent.Inside(Psample)) return false;
    // Compute the distance sampling PDF
    const Spectrum sigma_t = Sigma_t(Psample, ray.d, ray.time);
    Spectrum pdfSample = sigma_t * Exp(-sigma_t * t);
    *pdf = pdfSample.y();
    // Return true if _Psample_ is still inside the volume
    return true;
}


bool FluorescentScatteringVolumeDensity::SampleDistance(const Ray &ray, float *tDist,
        Point &Psample, float *pdf, RNG &rng, const int &wl) const {
    float t = -log(1 - rng.RandomFloat()) / Sigma_t(ray.o, ray.d, ray.time, wl);
    *tDist = ray.mint + t;
    Psample = ray(t);
    // Return false if _Psample_ is outside the volume
    if (!extent.Inside(Psample)) return false;
    // Compute the distance sampling PDF at _wl_
     const float sigma_t = Sigma_t(Psample, ray.d, ray.time, wl);
    *pdf = sigma_t * exp(-sigma_t * t);
    // Return true if _Psample_ is still inside the volume
    return true;
}


bool FluorescentScatteringVolumeDensity::SampleDirection(const Point &p, const Vector& wi,
        Vector& wo, float* pdf, RNG &rng) const {
    wo = SampleHG(wi, g, rng.RandomFloat(), rng.RandomFloat());
    *pdf = PhaseHG(wi, wo, g);
    return true;
}


FluorescentScatteringVolumeDensity
*CreateFluorescentScatteringVolumeDensityRegion(const Transform &volume2world,
        const ParamSet &params) {
    // Initialize common volume region parameters
    Spectrum sigma_a = params.FindOneSpectrum("sigma_a", 0.);
    Spectrum sigma_s = params.FindOneSpectrum("sigma_s", 0.);
    Spectrum fex = params.FindOneSpectrum("fex", 0.);
    Spectrum fem = params.FindOneSpectrum("fem", 0.);
    float molecularWeight = params.FindOneFloat("mweight", 0.);
    float epsilon = params.FindOneFloat("epsilon", 0.);
    float c = params.FindOneFloat("c", 0.);
    float yield = params.FindOneFloat("yield", 1.);
    float sscale = params.FindOneFloat("sscale", 1.);
    float fscale = params.FindOneFloat("fscale", 1.);
    float g = params.FindOneFloat("g", 0.);
    float gf = params.FindOneFloat("gf", 0.);
    float density = params.FindOneFloat("density", 1.);
    Spectrum Le = params.FindOneSpectrum("Le", 0.);
    Point p0 = params.FindOnePoint("p0", Point(0,0,0));
    Point p1 = params.FindOnePoint("p1", Point(1,1,1));

    return new FluorescentScatteringVolumeDensity(sigma_a, sigma_s, g, Le, BBox(p0, p1),
                volume2world, fex, fem, molecularWeight, epsilon, c,
                yield, gf, sscale, fscale, density);
}

}
