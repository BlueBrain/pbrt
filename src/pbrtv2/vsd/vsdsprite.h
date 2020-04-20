
/*
    pbrt source code Copyright(c) 2016 Marwan Abdellah.


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

#ifndef PBRT_VSD_VSDSPRITE_H
#define PBRT_VSD_VSDSPRITE_H

#include "geometry.h"
#include "vsdgrid.h"
#include "vsdevent.h"

namespace pbrt
{

// VSDSprite Declarations
class VSDSprite
{
public:
    // VSDSprite Public Methods
    VSDSprite() { }
    // @NOTE: This constructor requires computing the bounding box of the
    // sprite on the fly
    void Read(const string &datadirectory, const string &pshfile,
              const bool computeBoundingBox = false,
              const Vector &translation = Vector(0.f, 0.f, 0.f));
    // @NOTE: This constructor is faster since it loads the bounding box values
    // from the pre-computed configurations.
    void Read(const string &datadirectory, const string &pshfile,
              const Point &pMin, const Point &pMax, const Vector &dimensions,
              const Vector &shift = Vector(0.f, 0.f, 0.f));
    void Write(const string &datadirectory, const string &pshfile);
    void ComputeBoundingBox();
    void Shift(const Vector &shift);
    void CenterAtOrigin();
    void PrintBoundingBox() const;
    int GetNumberEvents() const;
    string GetTimeStep() const;
    float GetEventIntensity(int i) const;
    Point GetEventPosition(int i) const;
    void PrintBoundingBox( string outputDirectory, string pshFilePrefix ) const;
    VSDGrid* Voxelize(const int resolution = 512) const;
private:
    // VSDSprite Private Methods
    void ReadHeaderData(const string &hdrfile);
    void WriteHeaderData(const string &hdrfile);
private:
    // VSDSprite Private Data
    vector <VSDEvent> events;
    int eventsCount;
    Vector center;
    Vector dimensions;
    string positionFile, intensityFile;
    BBox bbox;
    string timeStep;
};

// Utilities
int ParseIntParameter(const string line);
float ParseFloatParameter(const std::string line);
std::string ParseStringParameter(const std::string line);

}
#endif // PBRT_VSD_VSDSPRITE_H
