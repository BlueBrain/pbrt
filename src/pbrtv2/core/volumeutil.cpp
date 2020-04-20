
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

// core/volumeutil.h*
#include "volumeutil.h"
#include "pbrt.h"
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <chrono>
#include <inttypes.h>
#include <omp.h>

using namespace std;
using namespace std::chrono;

namespace pbrt
{

void ReadHeader(const string &prefix, int &nx, int &ny, int &nz) {
    std::string header = prefix + std::string(".hdr");
    std::ifstream headerFile(header.c_str());
    if (headerFile.is_open()) {
        headerFile >> nx; headerFile >> ny; headerFile >> nz;
    } else {
        Error("The header file cannot be opened");
        nx = 0; ny = 0; nz = 0;
        return;
    }
    headerFile.close();
}


void ReadHeader(const string &prefix, uint64 &nx, uint64 &ny, uint64 &nz) {
    std::string header = prefix + std::string(".hdr");
    std::ifstream headerFile(header.c_str());
    if (headerFile.is_open()) {
        headerFile >> nx; headerFile >> ny; headerFile >> nz;
    } else {
        Error("The header file cannot be opened");
        nx = 0; ny = 0; nz = 0;
        return;
    }
    headerFile.close();
}


float* ReadFloatVolume(const std::string &prefix, uint64 &nx, uint64 &ny, uint64 &nz) {
    ReadHeader(prefix, nx, ny, nz);

    float* data = new float[nx*ny*nz];
    std::string volume = prefix + std::string(".img");

    // Read the volume data
    std::ifstream volumeFile(volume.c_str(), std::ios::in | std::ios::binary);
    if(volumeFile.is_open()) {
        // Get the length of the file
        volumeFile.seekg(0, std::ios::end);
        const uint64_t dataCount = volumeFile.tellg();
        if (dataCount != uint64_t(nx*ny*nz)) {
            Error("The volume size does not match the specified dimensions");
            return NULL;
        }
        volumeFile.clear();
        volumeFile.seekg(0);
        int counter = 0;
        while(volumeFile.tellg() != dataCount){
            unsigned char value;
            volumeFile.read((char*)&value, sizeof(unsigned char));
            data[counter] = float(value);
            counter++;
        }
    } else {
        Error("Cannot open the raw volume file %s\n", prefix.c_str());
        return NULL;
    }
    volumeFile.close();
    return data;
}


uint8* ReadIntVolume(const std::string &prefix, uint64 &nx, uint64 &ny, uint64 &nz) {
    ReadHeader(prefix, nx, ny, nz);

    using namespace std::chrono;
    high_resolution_clock::time_point start = high_resolution_clock::now();

    uchar* data = new uchar[nx*ny*nz];
    std::string volume = prefix + std::string(".img");

    // Read the volume data
    std::ifstream volumeFile(volume.c_str(), std::ios::in | std::ios::binary);
    if(volumeFile.is_open()) {
        // Get the length of the file
        volumeFile.seekg(0, std::ios::end);
        const uint64_t dataCount = volumeFile.tellg();
        if (dataCount != uint64_t(nx*ny*nz)) {
            Error("The volume size does not match the specified dimensions");
            return NULL;
        }
        volumeFile.clear();
        volumeFile.seekg(0);
        int counter = 0;
        while(volumeFile.tellg() != dataCount){
            unsigned char value;
            volumeFile.read((char*)&value, sizeof(unsigned char));
            data[counter] = value;
            counter++;
        }
    } else {
        Error("Cannot open the raw volume file %s\n", prefix.c_str());
        return NULL;
    }
    volumeFile.close();

    high_resolution_clock::time_point end = high_resolution_clock::now();

    duration<double> interval = duration_cast<duration<double>>(end - start);
    cout << "Loading volume in [" << interval.count() << "] seconds." << endl;

    return data;
}


BitArray* ReadBinaryVolume(const string &prefix, uint64 &nx, uint64 &ny, uint64 &nz) {
    ReadHeader(prefix, nx, ny, nz);
    uint64_t numElements = nx * ny * nz;
    high_resolution_clock::time_point start = high_resolution_clock::now();
    BitArray* volume = new BitArray(numElements);

    const string file = prefix + string(".bin");
    ifstream stream;
    stream.open(file.c_str(), ios::in | ios::binary);
    if(stream.fail()) {
        Error("Cannot open the fluorescence volume file [%s]\n", file.c_str());
    }
    stream.read((char*) volume->GetDataArray(), volume->GetNumBytes());
    stream.close();

    high_resolution_clock::time_point end = high_resolution_clock::now();

    duration<double> interval = duration_cast<duration<double>>(end - start);
    cout << "Loading volume in [" << interval.count() << "] seconds." << endl;

    return volume;
}


BitArray* ReadBinaryFluorescenceVolume(const string &prefix, uint64 &nx, uint64 &ny, uint64 &nz) {
    ReadHeader(prefix, nx, ny, nz);
    uint64_t numElements = uint64_t(nx) * uint64_t(ny) * uint64_t(nz);
    high_resolution_clock::time_point start = high_resolution_clock::now();
    BitArray* volume = new BitArray(numElements);

    const string file = prefix + string(".bin");
    ifstream stream;
    stream.open(file.c_str(), ios::in | ios::binary);
    if(stream.fail()) {
        Error("Cannot open the fluorescence volume file [%s]\n", file.c_str());
    }
    stream.read((char*) volume->GetDataArray(), volume->GetNumBytes());
    stream.close();

    high_resolution_clock::time_point end = high_resolution_clock::now();

    duration<double> interval = duration_cast<duration<double>>(end - start);
    cout << "Loading volume in [" << interval.count() << "] seconds." << endl;

    return volume;
}


void ReadVSDHeader(const string &prefix, uint64 &nx, uint64 &ny, uint64 &nz,
        float &p0x, float &p0y, float &p0z, float &p1x, float &p1y, float &p1z,
        float &maxValue) {
    std::string header = prefix + std::string(".hdr");
    std::ifstream headerFile(header.c_str(), std::ios::in);
    if (headerFile.is_open()) {
        headerFile >> nx; headerFile >> ny; headerFile >> nz;
        headerFile >> p0x; headerFile >> p0y; headerFile >> p0z;
        headerFile >> p1x; headerFile >> p1y; headerFile >> p1z;
        headerFile >> maxValue;
    } else {
        Error("The header file cannot be opened");
        nx = 0; ny = 0; nz = 0;
        return;
    }
    headerFile.close();
}


uint8* ReadIndices(const std::string &prefix, uint64& nx, uint64& ny, uint64& nz) {
    ReadHeader(prefix, nx, ny, nz);
    high_resolution_clock::time_point start = high_resolution_clock::now();
    uint8 *data = new uint8[nx*ny*nz];
    std::string volume = prefix + std::string(".img");
    ifstream stream;
    stream.open(volume.c_str(), ios::in | ios::binary);
    if(stream.fail()) {
        Error("Cannot open the volume file [%s]\n", volume.c_str());
    }
    size_t streamSize = nx * ny * nz;
    stream.read((char*) data, streamSize);
    stream.close();

    high_resolution_clock::time_point end = high_resolution_clock::now();

    duration<double> interval = duration_cast<duration<double>>(end - start);
    cout << "Loading volume in [" << interval.count() << "] seconds." << endl;

    return data;
}


float* ReadVSDVolume(const std::string &prefix, uint64& nx, uint64& ny, uint64& nz,
        float &p0x, float &p0y, float &p0z, float &p1x, float &p1y, float &p1z,
        float &maxValue) {

    high_resolution_clock::time_point start = high_resolution_clock::now();
    ReadVSDHeader(prefix, nx, ny, nz, p0x, p0y, p0z, p1x, p1y, p1z, maxValue);
    float* data = new float[nx*ny*nz];
    uint8* rawData = new uint8[nx*ny*nz];
    std::string volume = prefix + std::string(".raw");
    ifstream stream;
    stream.open(volume.c_str(), ios::in | ios::binary);
    if(stream.fail()) {
        Error("Cannot open the vsd volume file [%s]\n", volume.c_str());
    }
    size_t streamSize = nx * ny * nz;
    stream.read((char*) rawData, streamSize);
    stream.close();

    #pragma omp parallel for
    for(size_t i = 0; i < nx * ny * nz; i++) {
        data[i] = (10000.0/ 100.0) * rawData[i];
    }

    delete [] rawData;

    high_resolution_clock::time_point end = high_resolution_clock::now();

    duration<double> interval = duration_cast<duration<double>>(end - start);
    cout << "Loading volume in [" << interval.count() << "] seconds." << endl;
    return data;
}

}

