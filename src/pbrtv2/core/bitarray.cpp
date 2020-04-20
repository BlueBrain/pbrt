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

#include "bitarray.h"
#include <fstream>
#include <inttypes.h>

#define BIT_ARRAY_EXTENSION ".bin"
#define ASCII_EXTENSION ".ascii"


using namespace std;

namespace pbrt
{

BitArray::BitArray(const uint64_t &nelements) {
    numElements_ = nelements;
    numBytes_ = (uint64_t)
                float( numElements_ / 8.f ) + (( numElements_ % 8 ) ? 1 : 0);
    byteData_ = new uchar[numBytes_];
    for(uint64_t i = 0; i < numBytes_; i++)
        byteData_[i] = 0;
}


void BitArray::SetBit(const uint64_t &idx, const bool &value) {
    uint64_t byteIndex = idx / 8;
    uint64_t bitIndex = idx % 8;
    uchar byte = byteData_[byteIndex];
    UpdateByte(byte, bitIndex, value);
    byteData_[byteIndex] = byte;
}


bool BitArray::GetBit(const uint64_t &idx) const {
    uint64_t byteIndex = idx / 8;
    uint64_t bitIndex = idx % 8;
    uchar byte = byteData_[byteIndex];
    return (byte >> bitIndex) & 0x1;
}


uchar BitArray::GetByte(const uint64_t &idx) const {
    return byteData_[idx];
}


uchar* BitArray::GetDataArray() const {
    return byteData_;
}


void BitArray::SetDataArray(uchar* byteArray) {
    byteData_ = byteArray;
}


void BitArray::UpdateByte(uchar& byte, const uint64_t &bitIdx, const bool &value) {
    byte ^= (-value ^ byte) & (1 << bitIdx);
}


uint64_t BitArray::GetNumElements() const {
    return numElements_;
}


uint64_t BitArray::GetNumBytes() const {
    return numBytes_;
}


void BitArray::Write(const string &prefix) {
    const std::string filePath = prefix + BIT_ARRAY_EXTENSION;
    ofstream stream(filePath.c_str(), ios::out | ios::binary);
    for(uint64_t i = 0; i < numBytes_; i++)
        stream << byteData_[i];
    stream.close();
}


void BitArray::WriteASCII(const string &prefix) {
    const std::string filePath = prefix + ASCII_EXTENSION;
    ofstream stream(filePath.c_str(), ios::out);
    uint64_t count = 0;
    for(uint64_t i = 0; i < numBytes_; i++)
    {
        if(byteData_[i] > 0)
        {
            count++;
            printf("%lu [%d] %lu \n", i, byteData_[i], count);
        }
    }
    for(uint64_t i = 0; i < numElements_; i++)
    {
        if(GetBit(i))
            stream << "1";
        else
            stream << "0";
        stream << " ";
    }
    stream.close();
}


BitArray::~BitArray() {
    delete [] byteData_;
}

}

