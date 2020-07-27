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

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PBRT_LIGHTS_COLLIMATED_WITH_ANGLE_H
#define PBRT_LIGHTS_COLLIMATED_WITH_ANGLE_H

// lights/collimated.h*
#include "pbrt.h"
#include "light.h"
#include "primitive.h"

// CollimatedAreaLightWithAngle Declarations
class CollimatedAreaLightWithAngle : public AreaLight {
public:
    // CollimatedAreaLightWithAngle Public Methods
    CollimatedAreaLightWithAngle(const Transform &light2world, const LightUnit &unit,
        const Spectrum &photons, int ns, const Reference<Shape> &shape, float angle);
    ~CollimatedAreaLightWithAngle();
    Spectrum L(const Point &p, const Normal &n, const Vector &w) const {
        return Dot(n, w) > 0.f ? Lemit : 0.f;
    }
    Spectrum Power(const Scene *) const { return Pemit; }
    Spectrum Irradiance(const Scene *) const { return Eemit; }
    Spectrum Radiance(const Scene *) const { return Lemit; }
    bool IsDeltaLight() const { return false; }
    float Pdf(const Point &, const Vector &) const;
    Spectrum Sample_L(const Point &P, float pEpsilon, const LightSample &ls, float time,
        Vector *wo, float *pdf, VisibilityTester *visibility) const;
    Spectrum Sample_L(const Scene *scene, const LightSample &ls, float u1, float u2,
        float time, Ray *ray, Normal *Ns, float *pdf) const;
    void PerformLaserTest();
protected:
    // CollimatedAreaLightWithAngle Protected Data
    Spectrum Pemit, Eemit, Lemit;
    LightUnit lightUnit;
    ShapeSet *shapeSet;
    float area;
    float cosThetaMaximum;
    float thetaMax;
};


AreaLight
*CreateCollimatedAreaLightWithAngle(const Transform &light2world, const ParamSet &paramSet,
        const Reference<Shape> &shape);

#endif // PBRT_LIGHTS_COLLIMATED_H
