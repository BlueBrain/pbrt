
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


// volumes/fluorescentgrid.cpp*
#include "stdafx.h"
#include "fluorescentgrid.h"
#include "volumeutil.h"
#include "paramset.h"
#include <limits>
#include <fstream>
#include <math.h>
#include "montecarlo.h"

namespace pbrt
{

// FluorescentGridDensity Method Definitions
void FluorescentGridDensity::ValidateData() {
    // Write the spectra to validate the input data
    f_ex.WriteSPD("fex");
    f_em.WriteSPD("fem");
    mu.WriteSPD("sigma");
}


float FluorescentGridDensity::F(int x, int y, int z) const {
    x = Clamp(x, 0, nx-1);
    y = Clamp(y, 0, ny-1);
    z = Clamp(z, 0, nz-1);
    if (grid[z*nx*ny + y*nx + x])
        return 1.f;
    return 0.f;
}


float FluorescentGridDensity::Fluorescence(const Point &Pobj) const {
    if (!extent.Inside(Pobj)) return 0.f;
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


Spectrum FluorescentGridDensity::STER(const Point &p, const Vector &w,
        float time) const {
    Severe("Unimplemented FluorescentGridDensity::STER() method called");
    return false;
}


Spectrum FluorescentGridDensity::ATER(const Point &p, const Vector &w,
        float time) const {
    Severe("Unimplemented FluorescentGridDensity::ATER() method called");
    return false;
}


float FluorescentGridDensity::STER(const Point &p, const Vector &w, float time,
        const int &wl) const {
    Severe("Unimplemented FluorescentGridDensity::STER() method called");
    return false;
}


float FluorescentGridDensity::ATER(const Point &p, const Vector &w, float time,
        const int &wl) const {
    Severe("Unimplemented FluorescentGridDensity::ATER() method called");
    return false;
}

FluorescentGridDensity *CreateFluorescentGrid(const Transform &volume2world,
        const ParamSet &params) {
    // Initialize common volume region parameters
    Spectrum fex = params.FindOneSpectrum("fex", 0.);
    Spectrum fem = params.FindOneSpectrum("fem", 0.);
    float epsilon = params.FindOneFloat("epsilon", 0.);
    float c = params.FindOneFloat("c", 0.);
    float yield = params.FindOneFloat("yield", 1.);
    float gf = params.FindOneFloat("gf", 0.);
    Point p0 = params.FindOnePoint("p0", Point(-0.5, -0.5, -0.5));
    Point p1 = params.FindOnePoint("p1", Point(0.5, 0.5, 0.5));
    std::string format = params.FindOneString("format", "raw");
    uchar* data;
    if (format == std::string("raw")) {
        std::string prefix = params.FindOneString("prefix", "");
        Info("Reading a RAW volume from %s \n", prefix.c_str());
        u_int64_t nx, ny, nz;
        data = ReadIntVolume(prefix, nx, ny, nz);
        return new FluorescentGridDensity(BBox(p0, p1), volume2world,
                nx, ny, nz, data, fex, fem, epsilon, c, yield, gf);
    } else {
        int nitems;
        const uchar *data = params.FindUChar("density", &nitems);
        if (!data) {
            Error("No \"density\" values provided for volume grid?");
            return NULL;
        }
        int nx = params.FindOneInt("nx", 1);
        int ny = params.FindOneInt("ny", 1);
        int nz = params.FindOneInt("nz", 1);
        if (nitems != nx*ny*nz) {
            Error("FluorescentGridDensity has %d density values but nx*ny*nz = %d",
                  nitems, nx*ny*nz);
            return NULL;
        }
        return new FluorescentGridDensity(BBox(p0, p1), volume2world,
                nx, ny, nz, data, fex, fem, epsilon, c, yield, gf);
    }
}

}
