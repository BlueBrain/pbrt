
/*
    pbrt source code Copyright(c) 2015 Marwan Abdellah.

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

#ifndef PBRT_CORE_VOLUMEUTIL_H
#define PBRT_CORE_VOLUMEUTIL_H

#include "bitarray.h"

namespace pbrt
{

void ReadHeader(const std::string &prefix, uint64 &nx, uint64 &ny, uint64 &nz);
uint8* ReadIntVolume(const std::string &prefix, uint64 &nx, uint64 &ny, uint64 &nz);
float* ReadFloatVolume(const std::string &prefix, uint64 &nx, uint64 &ny, uint64 &nz);
uint8* ReadIndices(const std::string &prefix, uint64& nx, uint64& ny, uint64& nz);
void ReadColorTaggedHeader(const std::string &prefix, int &nx, int &ny, int &nz,
        int &ntags, float *density, Spectrum *sa, Spectrum *ss, Spectrum *le);
void ReadColorTaggedVolume(const std::string &prefix, int &nx, int &ny, int &nz,
        int &ntags, float *density, Spectrum *sa, Spectrum *ss, Spectrum *le);
BitArray* ReadBinaryVolume(const string &prefix, uint64 &nx, uint64 &ny, uint64 &nz);
BitArray* ReadBinaryFluorescenceVolume(const string &prefix, uint64 &nx, uint64 &ny,
    uint64 &nz);
void ReadVSDHeader(const std::string &prefix, int &nx, int &ny, int &nz,
    float &p0x, float &p0y, float &p0z, float &p1x, float &p1y, float &p1z,
    float &maxValue);
float* ReadVSDVolume(const std::string &prefix, uint64 &nx, uint64 &ny, uint64 &nz,
    float &p0x, float &p0y, float &p0z, float &p1x, float &p1y, float &p1z,
    float &maxValue);

}

#endif // PBRT_CORE_VOLUMEUTIL_H
