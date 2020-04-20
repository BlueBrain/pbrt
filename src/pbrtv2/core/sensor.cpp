
/*
    pbrt source code Copyright(c) 2015-2016
    Marwan Abdellah <marwan.abdellah@epfl.ch>


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


// core/sensor.cpp*
#include "stdafx.h"
#include "sensor.h"
#include "paramset.h"
#include "imageio.h"
#include <typeinfo>
#include <fstream>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <cmath>

using namespace std;

namespace pbrt
{

// Shape Method Definitions
Sensor::~Sensor() {
    delete [] Lpixels;
}


Sensor::Sensor(const std::string shapeid, const std::string shaperef,
               const Shape* surface, const uint64_t xres, const uint64_t yres,
               const float widthum, const float heightum, const float fov)
    : surfaceShape(shapeid), reference(shaperef), shape(surface),
      xPixels(xres), yPixels(yres),
      xLength_um(widthum), yLength_um(heightum), fieldOfView(fov) {
    pixelArea = xres * yres;
    area_um2 = widthum * heightum;
    hitCount = 0;
    Lpixels = new Spectrum[pixelArea];
    recordedEnergy = new float[pixelArea];
    for (uint64_t i = 0; i < pixelArea; i++) {
        Lpixels[i] = Spectrum(0.f);
        recordedEnergy[i] = 0.f;
    }
}


bool Sensor::Intersect(const Ray &ray, float *tHit) {
    DifferentialGeometry dgLight;
    float rayEpsilon;
    if (shape->Intersect(ray, tHit, &rayEpsilon, &dgLight)) {
        // Obtain the normal on the surface.
        Normal surfaceNormal;
        shape->Sample(0.f, 0.f, &surfaceNormal);

        // Make sure that the ray is coming in the direction of the surface
        // of the sensor.
        const float theta = Degrees(acos(Dot(-surfaceNormal, ray.d)));
        if (abs(theta) >= 0.f && abs(theta) < 90.0)
            return true;
    }
    return false;
}


bool Sensor::Hit(const Ray &ray, float *tHit, float tMax) {
    if (!Intersect(ray, tHit))
        return false;
    if (tMax < *tHit)
        return false;
    return true;
}


void Sensor::RecordHit(const Point& point, const Spectrum& energy) {
    hitCount++;

    // Translate the point to the object space.
    Point Pobj = (*shape->WorldToObject)(point);

    // Compute the relative offsets of the points on the sensor.
    BBox sensorExtent = shape->ObjectBound();
    const float xDistance = (Pobj.x - sensorExtent.pMin.x);
    const float xSensorLength = (sensorExtent.pMax.x - sensorExtent.pMin.x);
    const float xRelativeOffset = xDistance / xSensorLength;
    const float yDistance = (Pobj.y - sensorExtent.pMin.y);
    const float ySensorLength = (sensorExtent.pMax.y - sensorExtent.pMin.y);
    const float yRelativeOffset = yDistance / ySensorLength;

    // Find the corresponding pixel.
    const uint64_t xPixel = Floor2Int(xRelativeOffset * xPixels);
    const uint64_t yPixel = Floor2Int(yRelativeOffset * yPixels);
    const uint64_t index = xPixel + xPixels * yPixel;

    // Add the energy contribution to the pixel.
    Lpixels[index] += energy;
    recordedEnergy[index] += energy.y();
}


float Sensor::SurfaceArea(void) const {
    return shape->Area();
}


std::string Sensor::ReferenceString(void) const {
    return reference;
}

uint64_t Sensor::NumberPixelsX(void) const {
    return xPixels;
}


uint64_t Sensor::NumberPixelsY(void) const {
    return yPixels;
}


uint64_t Sensor::NumberPixels(void) const {
    return pixelArea;
}


float Sensor::FilmWidth_um(void) const {
    return xLength_um;
}


float Sensor::FilmHeight_um(void) const {
    return yLength_um;
}


float Sensor::FilmArea_um2(void) const {
    return area_um2;
}


float Sensor::FOV(void) const {
    return fieldOfView;
}


Spectrum* Sensor::FilmRadiance(void) const {
    return Lpixels;
}


uint64_t Sensor::HitCount(void) const {
    return hitCount;
}


void Sensor::WriteFilm(void) {
    float *rgb = new float[3*pixelArea];
    int offset = 0;
    int index = 0;
    for (uint64_t y = 0; y < yPixels; ++y) {
        for (uint64_t x = 0; x < xPixels; ++x) {
            index = (xPixels-x) + xPixels*y;
            // Convert the spectrum to XYZ
            float xyz[3];
            Lpixels[index].ToXYZ(xyz);

            // Convert pixel XYZ color to RGB
            XYZToRGB(xyz, &rgb[3*offset]);
            index++; offset++;
        }
    }

    // Write an RGB image
    const string sensorImageName = reference + ".exr";
    WriteImage(sensorImageName, rgb, NULL, xPixels, yPixels,
               xPixels, yPixels, 0, 0);

    // Write the VSD image
    const string file =  reference + ".vsd";
    fstream stream(file.c_str(), ios::out);
    for (uint64_t x = 0; x < xPixels; ++x)
        for (uint64_t y = 0; y < yPixels; ++y)
            stream << x << " " << y << " "
                   << recordedEnergy[x + xPixels* y] << endl;
    stream.close();

    delete [] rgb;
}


Sensor* CreateSensor(const std::string shapeId, Shape* shape,
                     const ParamSet &params)
{
    Sensor* sensor = 0;
    const string reference = params.FindOneString("reference", "sensor");
    const uint64_t xPixels = params.FindOneInt("xpixels", 0);
    const uint64_t yPixels = params.FindOneInt("ypixels", 0);
    const float fov = params.FindOneFloat("fov", 45.f);

    if (shapeId == std::string("disk")) {
        float radius = params.FindOneFloat("radius", 0.f);
        sensor = new Sensor(shapeId, reference, shape,
                            xPixels, yPixels, 2 * radius, 2 * radius, fov);
    } else if (shapeId == std::string("rectangle")) {
        float x = params.FindOneFloat("x", 0.f);
        float y = params.FindOneFloat("y", 0.f);
        sensor = new Sensor(shapeId, reference, shape,
                            xPixels, yPixels, x, y, fov);
    } else {
        Error("The sensor is defined for disk and rectangle shapes only");
        exit(0);
    }

    return sensor;
}

}
