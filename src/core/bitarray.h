
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

#ifndef PBRT_CORE_BITARRAY_H
#define PBRT_CORE_BITARRAY_H

#include "pbrt.h"

namespace pbrt
{

class BitArray
{
public:
    BitArray(const uint64_t &nelements);
    ~BitArray();
    void SetBit(const uint64_t &idx, const bool &value);
    bool GetBit(const uint64_t &idx) const;
    uchar GetByte(const uint64_t &idx) const;
    uchar* GetDataArray() const;
    void SetDataArray(uchar* byteArray);
    uint64_t GetNumElements() const;
    uint64_t GetNumBytes() const;
    void Write(const string &prefix);
    void WriteASCII(const string &prefix);
private:
    void UpdateByte(uchar& byte, const uint64_t &bitIdx, const bool &value);
private:
    uint64_t numElements_;
    uint64_t numBytes_;
    uchar* byteData_;
};

}

#endif // PBRT_CORE_BITARRAY_H

