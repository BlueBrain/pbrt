
/*
    pbrt source code Copyright(c) 2012-2016 Marwan Abdellah.

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


// volumes/fluorescentannotatedgrid.cpp*
#include "stdafx.h"
#include "fluorescentannotatedgrid.h"
#include "volumeutil.h"
#include "paramset.h"
#include <limits>
#include <fstream>
#include <sstream>
#include <math.h>
#include "montecarlo.h"

using namespace std;

namespace pbrt
{

// FluorescentAnnotatedVolumeGrid Method Definitions
void FluorescentAnnotatedVolumeGrid::ValidateData() const {
    for (uint64 i = 0; i < nTags; ++i) {
        stringstream stream;
        stream << "fex_" << i;
        f_ex[i].WriteSPD(stream.str().c_str());
        stream.str(std::string());
        stream << "fem_" << i;
        f_em[i].WriteSPD(stream.str().c_str());
        stream.str(std::string());
        stream << "sigma_" << i;
        mu[i].WriteSPD(stream.str().c_str());
    }
}


uint64 FluorescentAnnotatedVolumeGrid::Index(const Point &Pobj) const {
    if (!extent.Inside(Pobj)) return 0;
    // Compute voxel coordinates and offsets for _Pobj_
    Vector vox = extent.Offset(Pobj);
    vox.x = vox.x * nx;
    vox.y = vox.y * ny;
    vox.z = vox.z * nz;
    uint64 vx = Floor2Int(vox.x), vy = Floor2Int(vox.y), vz = Floor2Int(vox.z);
    uint64 i = vx + nx * vy + nx * ny * vz;
    if (i > 0 && i < nx * ny * nz)
        return indices[i];
    return 0;
}


FluorescentAnnotatedVolumeGrid *CreateFluorescentAnnotatedVolumeGrid(const Transform &volume2world,
        const ParamSet &params) {
    // Initialize common volume region parameters
    float g = params.FindOneFloat("g", 0.);
    Point p0 = params.FindOnePoint("p0", Point(-0.5, -0.5, -0.5));
    Point p1 = params.FindOnePoint("p1", Point(0.5, 0.5, 0.5));
    int numberTags = params.FindOneInt("ntags", 1);
    std::string prefix = params.FindOneString("prefix", "");
    Spectrum* fex = new Spectrum[numberTags];
    Spectrum* fem = new Spectrum[numberTags];
    float *epsilon = new float[numberTags];
    float *c = new float[numberTags];
    float *yield = new float[numberTags];
    float *gf = new float[numberTags];
    for (int i = 1; i <= numberTags; i++) {
        std::stringstream stream;
        stream << "fex_" << i;
        fex[i-1] = params.FindOneSpectrum(stream.str(), 0.);
        stream.str(string(""));

        stream << "fem_" << i;
        fem[i-1] = params.FindOneSpectrum(stream.str(), 0.);
        stream.str(string(""));

        stream << "epsilon_" << i;
        epsilon[i-1] = params.FindOneFloat(stream.str(), 0.);
        stream.str(string(""));

        stream << "c_" << i;
        c[i-1] = params.FindOneFloat(stream.str(), 0.);
        stream.str(string(""));

        stream << "yield_" << i;
        yield[i-1] = params.FindOneFloat(stream.str(), 0.);
        stream.str(string(""));

        stream << "gf_" << i;
        gf[i-1] = params.FindOneFloat(stream.str(), 0.);
        stream.str(string(""));
    }

    uint64 nx, ny, nz;
    uint8 *indices = ReadIndices(prefix, nx, ny, nz);
    return new FluorescentAnnotatedVolumeGrid(BBox(p0, p1), volume2world,
            fex, fem, epsilon, c, yield, gf, nx, ny, nz, indices, numberTags);
}

}
