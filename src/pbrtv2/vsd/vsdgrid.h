
/*
    pbrt source code Copyright(c) 2016 Marwan Abdellah.
                                  Blue Brain Project (BBP) / EPFL


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

#ifndef PBRT_VSD_VSDGRID_H
#define PBRT_VSD_VSDGRID_H

// vsd/volumegrid.h*
#include "geometry.h"
#include "vsdevent.h"

namespace pbrt
{

// VSDGrid Declarations
class VSDGrid
{
public:
    // VSDGrid Public Methods
    VSDGrid(const int &nxx, const int &nyy, const int &nzz, const BBox bb);
    void AddEvent(const VSDEvent *event);
    void GeneratePBRTConfiguration(const string &outputDir, const string &prefix,
            const std::string &simulationMethod, const int &gridBaseResolution,
            const int &sensorBaseResolution, const Point &sensorPosition,
            const Vector &sensorDimensions, const Vector &spriteTranslation);
    void Shift(const float &x, const float &y, const float &z);
    void WritePBRTVolumeFile(const string &outputDir, const string &prefix,
            const Vector &translation);
    void WriteRAWVolumeFile(const string &outputDir, const string &prefix,
            const Vector &translation);
    float* Data() { return density; }
private:
    // VSDGrid Private Data
    float *density;
    int nx, ny, nz, nxyz;
    float dx, dy, dz;
    BBox bbox;
};
}
#endif // PBRT_VSD_VSDGRID_H
