
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


// volumes/annotatedgrid.cpp*
#include "stdafx.h"
#include "annotatedgrid.h"
#include "volumeutil.h"
#include "paramset.h"
#include <limits>
#include <fstream>
#include <sstream>
#include <math.h>
#include "montecarlo.h"

namespace pbrt
{

// AnnotatedVolumeGrid Method Definitions
float AnnotatedVolumeGrid::Density(const Point &Pobj) const {
    if (!extent.Inside(Pobj)) return 0;
    int tagIndex = Index(Pobj);
    if(tagIndex > 0)
        return density[tagIndex-1];
    return 0.0;
}


uint64 AnnotatedVolumeGrid::Index(const Point &Pobj) const {
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


AnnotatedVolumeGrid *CreateAnnotatedVolumeGrid(const Transform &volume2world,
        const ParamSet &params) {
    // Initialize common volume region parameters
    float g = params.FindOneFloat("g", 0.);
    Point p0 = params.FindOnePoint("p0", Point(-0.5, -0.5, -0.5));
    Point p1 = params.FindOnePoint("p1", Point(0.5, 0.5, 0.5));
    int numberTags = params.FindOneInt("ntags", 1);
    std::string prefix = params.FindOneString("prefix", "");

    float *density = new float[numberTags];
    Spectrum* sig_a = new Spectrum[numberTags];
    Spectrum* sig_s = new Spectrum[numberTags];
    Spectrum* le = new Spectrum[numberTags];

    for (int i = 1; i <= numberTags; i++) {
        std::stringstream stream_a, stream_s, stream_le, stream_d;
        stream_a << "sigma_a_" << i;
        sig_a[i-1] = params.FindOneSpectrum(stream_a.str(), 0.);
        stream_s << "sigma_s_" << i;
        sig_s[i-1] = params.FindOneSpectrum(stream_s.str(), 0.);
        stream_le << "Le_" << i;
        le[i-1] = params.FindOneSpectrum(stream_le.str(), 0.);
        stream_d << "density_" << i;
        density[i-1] = params.FindOneFloat(stream_d.str(), 0.);
    }

    uint64 nx, ny, nz;
    uint8 *indices = ReadIndices(prefix, nx, ny, nz);
    return new AnnotatedVolumeGrid(sig_a, sig_s, g, le, BBox(p0, p1),
                volume2world, nx, ny, nz, density, indices, numberTags);
}
}
