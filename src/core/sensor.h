
/*
    pbrt source code Copyright(c) Marwan Abdellah <marwan.abdellah>@epfl.ch>.

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

#ifndef PBRT_CORE_SENSOR_H
#define PBRT_CORE_SENSOR_H

// core/sensor.h*
#include "pbrt.h"
#include "geometry.h"
#include "transform.h"
#include "diffgeom.h"
#include "memory.h"
#include "shape.h"

namespace pbrt
{

class Sensor : public ReferenceCounted
{
public:
    Sensor(const std::string shapeid, const std::string shaperef,
           const Shape* surface, const uint64_t xres, const uint64_t yres,
           const float widthum, const float heightum, const float fov);

    bool Intersect(const Ray &ray, float *tHit);
    bool Hit(const Ray &ray, float *tHit, float tMax);
    void RecordHit(const Point& point, const Spectrum& energy);
    std::string ReferenceString(void) const;
    float SurfaceArea(void) const;
    uint64_t NumberPixelsX(void) const;
    uint64_t NumberPixelsY(void) const;
    uint64_t NumberPixels(void) const;
    float FilmWidth_um(void) const;
    float FilmHeight_um(void) const;
    float FilmArea_um2(void) const;
    float FOV(void) const;
    Spectrum* FilmRadiance(void) const;
    uint64_t HitCount(void) const;
    void WriteFilm(void);

    ~Sensor();

private:
    const std::string surfaceShape; // The shape of the surface.
    const std::string reference; // Reference name to the sensor.
    const Shape* shape; // Sensor shape object.
    uint64_t hitCount; // Total number of hits recorded/
    const uint64_t xPixels; // Number of pixels in x.
    const uint64_t yPixels; // Number of pixels in y.
    uint64_t pixelArea; // Total number of pixels in the sensor.
    const float xLength_um; // Sensor surface length in um.
    const float yLength_um; // Sensor surface height in um.
    float area_um2; // Sensor surface area in um2.
    const float fieldOfView; // Sensor field of view.
    Spectrum* Lpixels; // Radiance distribution recorded by the sensor.
    float* recordedEnergy; // Energy recorded by the sensor.
};

Sensor* CreateSensor(const std::string shapeid, Shape* shape, const ParamSet &params);

}

#endif // PBRT_CORE_SENSOR_H
