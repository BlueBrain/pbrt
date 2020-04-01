
/*
    pbrt source code Copyright(c) 1998-2012 Matt Pharr and Greg Humphreys.
                                  2012-2016 Marwan Abdellah.

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

#ifndef PBRT_VOLUMES_FLUORESCENTBINARYVOLUMEGRID_H
#define PBRT_VOLUMES_FLUORESCENTBINARYVOLUMEGRID_H

// volumes/fluorescentbinarygrid.h*
#include "volume.h"
#include "fluorescentgrid.h"
#include <vector>
#include "bitarray.h"

namespace pbrt
{

// FluorescentBinaryVolumeGrid Declarations
class FluorescentBinaryVolumeGrid : public FluorescentGridDensity {
public:
    // FluorescentBinaryVolumeGrid Public Methods
    FluorescentBinaryVolumeGrid(const BBox &e, const Transform &v2w,
            int x, int y, int z, BitArray *data, const Spectrum &fex,
            const Spectrum &fem, float eps, float conc, float phi, float ggf)
        : FluorescentGridDensity(e, v2w, x, y, z, 0, fex, fem, eps, conc,
            phi, ggf) { grid = data; }
    ~FluorescentBinaryVolumeGrid() {
        grid->~BitArray();
    }
    float F(int x, int y, int z) const;
private:
    // FluorescentBinaryVolumeGrid Private Data
    BitArray *grid;
};


FluorescentBinaryVolumeGrid *CreateFluorescentBinaryVolumeGrid
(const Transform &volume2world, const ParamSet &params);

}

#endif // PBRT_VOLUMES_FLUORESCENTBINARYVOLUMEGRID_H
