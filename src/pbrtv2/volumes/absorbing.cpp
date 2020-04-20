
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


// volumes/absorbing.cpp*
#include "stdafx.h"
#include "absorbing.h"
#include "paramset.h"
#include "montecarlo.h"

namespace pbrt
{

// AbsorbingVolume Method Definitions
void AbsorbingVolume::ValidateData() const {
    mu.WriteSPD("sigma");
}


AbsorbingVolume
*CreateAbsorbingVolumeRegion(const Transform &volume2world,
        const ParamSet &params) {
    // Initialize common volume region parameters
    float epsilon = params.FindOneFloat("epsilon", 0.);
    float c = params.FindOneFloat("c", 0.);
    float g = params.FindOneFloat("g", 0.);
    Point p0 = params.FindOnePoint("p0", Point(-0.5, -0.5, -0.5));
    Point p1 = params.FindOnePoint("p1", Point(0.5, 0.5, 0.5));
    return new AbsorbingVolume(BBox(p0, p1), volume2world, epsilon, c, g);
}

}
