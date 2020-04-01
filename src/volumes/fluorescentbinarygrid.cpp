
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


// volumes/fluorescentbinarygrid.cpp*
#include "stdafx.h"
#include "volumes/fluorescentbinarygrid.h"
#include "fluorescentbinarygrid.h"
#include "grid.h"
#include "paramset.h"
#include <limits>
#include <fstream>
#include <math.h>
#include "montecarlo.h"
#include "volumeutil.h"

namespace pbrt
{
// FluorescentBinaryVolumeGrid Method Definitions
float FluorescentBinaryVolumeGrid::F(int x, int y, int z) const {
    x = Clamp(x, 0, nx-1);
    y = Clamp(y, 0, ny-1);
    z = Clamp(z, 0, nz-1);
    return ((grid->GetBit(z*nx*ny + y*nx + x)) ? 1.f : 0.f);
}


FluorescentBinaryVolumeGrid
*CreateFluorescentBinaryVolumeGrid(const Transform &volume2world,
        const ParamSet &params) {
    Spectrum fex = params.FindOneSpectrum("fex", 0.);
    Spectrum fem = params.FindOneSpectrum("fem", 0.);
    float epsilon = params.FindOneFloat("epsilon", 0.);
    float c = params.FindOneFloat("c", 0.);
    float yield = params.FindOneFloat("yield", 1.);
    float gf = params.FindOneFloat("gf", 0.);
    Point p0 = params.FindOnePoint("p0", Point(-0.5, -0.5, -0.5));
    Point p1 = params.FindOnePoint("p1", Point(0.5, 0.5, 0.5));
    std::string prefix = params.FindOneString("prefix", "");
    u_int64_t nx, ny, nz;
    BitArray* density = ReadBinaryVolume(prefix, nx, ny, nz);
    return new FluorescentBinaryVolumeGrid(BBox(p0, p1), volume2world,
        nx, ny, nz, density, fex, fem, epsilon, c, yield, gf);
}
}
