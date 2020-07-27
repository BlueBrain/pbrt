/*
    pbrt source code Copyright(c) 1998-2012 Matt Pharr and Greg Humphreys.

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


// lights/collimated.cpp*
#include "stdafx.h"
#include "lights/collimatedwithangle.h"
#include "paramset.h"
#include "montecarlo.h"
#include <string>
#include <sstream>

using namespace std;

// CollimatedAreaLightWithAngle Method Definitions
CollimatedAreaLightWithAngle::~CollimatedAreaLightWithAngle() {
    delete shapeSet;
}


CollimatedAreaLightWithAngle::CollimatedAreaLightWithAngle(const Transform &light2world,
        const LightUnit &unit, const Spectrum &photons, int ns,
        const Reference<Shape> &s, float angle)
    : AreaLight(light2world, ns) {
    shapeSet = new ShapeSet(s);
    area = shapeSet->Area();
    lightUnit = unit;
    cosThetaMaximum = cosf(Radians(angle));
    thetaMax = angle;

    // Verify a valid illumination input
    if (photons.IsBlack()) {
        Warning("The light source doesn't emit any power");
    }

    // Compute radiometric units
    switch (lightUnit) {
    case LightUnit::Power:
        Pemit = photons;
        Eemit = Pemit / area;
        Lemit = Eemit / M_PI;
        break;
    case LightUnit::Irradiance:
        Eemit = photons;
        Pemit = Eemit * area;
        Lemit = Eemit / M_PI;
        break;
    case LightUnit::Radiance:
    default:
        Lemit = photons;
        Eemit = Lemit * M_PI;
        Pemit = Eemit * area;
        break;
    }

    PerformLaserTest();

    // Validate light info
    FILE* lightInfo;
    static int i = 0;
    stringstream lightStream;
    lightStream << "light." << i << ".info"; i++;
    lightInfo = fopen(lightStream.str().c_str(), "w"); {
        float totalRadiance = Lemit.CalculateSPDPower();
        float totalIrradiance = totalRadiance * M_PI;
        float totalPower = totalIrradiance * area;
        fprintf(lightInfo, "Light area %f \n", area);
        fprintf(lightInfo, "Total L (Radiance) %e \n", totalRadiance);
        fprintf(lightInfo, "Total E (Irradiance) %e \n", totalIrradiance);
        fprintf(lightInfo, "Total Flux (Power) %e \n", totalPower);
        if(isLaser) fprintf(lightInfo, "Laser %d", laserWavelength);
        fprintf(lightInfo, "\n\nEmission spectrum\n\n");
        Lemit.WriteWavelengthIntensity(lightInfo);
    }
    fclose(lightInfo);
}


void CollimatedAreaLightWithAngle::PerformLaserTest() {
    isLaser = Lemit.VerifyLaser( laserWavelength, laserWavelengthIndex );
}


Spectrum CollimatedAreaLightWithAngle::Sample_L(const Point &p, float pEpsilon,
        const LightSample &ls, float time, Vector *wi, float *pdf,
        VisibilityTester *visibility) const {
    PBRT_AREA_LIGHT_STARTED_SAMPLE();
    Normal ns;
    Point ps;

    // Make sure that the point projects the light source surface area
    if(!shapeSet->Projects(p, ps, ns)) {
        return 0.f;
    }

    *wi = Normalize(ps - p);
    *pdf = shapeSet->Pdf(p);
    visibility->SetSegment(p, pEpsilon, ps, 1e-3f, time);
    Spectrum Ls = L(ps, ns, -*wi);
    PBRT_AREA_LIGHT_FINISHED_SAMPLE();
    return Ls;
}


float CollimatedAreaLightWithAngle::Pdf(const Point &p, const Vector &wi) const {
    Point ps;
    Normal ns;
    if (shapeSet->Projects(p, ps, ns))
        return shapeSet->Pdf(p, wi);
    return 0.f;
}


Spectrum CollimatedAreaLightWithAngle::Sample_L(const Scene *scene,
        const LightSample &ls, float u1, float u2, float time,
        Ray *ray, Normal *Ns, float *pdf) const {
    PBRT_AREA_LIGHT_STARTED_SAMPLE();
    Point org = shapeSet->Sample(ls, Ns);
    Vector dir;
    dir.x = Ns->x; dir.y = Ns->y; dir.z = Ns->z;
    // UniformSampleSphere(u1, u2);
    dir = UniformSampleCone(u1, u2, thetaMax);
    if (Dot(dir, *Ns) < 0.) dir *= -1.f;
    *ray = Ray(org, dir, 1e-3f, INFINITY, time);
    *pdf = shapeSet->Pdf(org) * INV_TWOPI;
    Spectrum Ls = L(org, *Ns, dir);
    PBRT_AREA_LIGHT_FINISHED_SAMPLE();
    return Ls;
}


AreaLight
*CreateCollimatedAreaLightWithAngle(const Transform &light2world, const ParamSet &paramSet,
        const Reference<Shape> &shape) {
    Spectrum photons = paramSet.FindOneSpectrum("photons", Spectrum(0.0));
    string units = paramSet.FindOneString("units", "radiance");
    Spectrum sc = paramSet.FindOneSpectrum("scale", Spectrum(1.0));
    int nSamples = paramSet.FindOneInt("nsamples", 1);
    float angle = paramSet.FindOneFloat("angle", 45.0);
    LightUnit lightUnit;
    if (units == "power")
        lightUnit = Power;
    else if (units == "irradiance")
        lightUnit = Irradiance;
    else
        lightUnit = Radiance;
    if (PbrtOptions.quickRender) nSamples = max(1, nSamples / 4);
    return new CollimatedAreaLightWithAngle(light2world, lightUnit, photons * sc,
                    nSamples, shape, angle);
}
