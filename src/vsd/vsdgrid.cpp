
/*
    pbrt source code Copyright(c) 2016 Marwan Abdellah.

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

#include "vsdgrid.h"
#include <fstream>
#include <string>
#include <limits>

#define N "\n"
#define NN "\n\n"
#define SIGMA_S_CORTEX 0.0180   /// 18.0 1/mm
#define SIGMA_A_CORTEX 0.0002   /// 0.20 1/mm

using namespace std;

VSDGrid::VSDGrid(const int &nxx, const int &nyy, const int &nzz,
                 const BBox bb) :
    nx(nxx), ny(nyy), nz(nzz), nxyz(nx * ny * nz), bbox(bb)
{
    // Allocate the volume grid and initialize it
    density = new float[nxyz];
    for(uint64_t i = 0; i < nxyz; i++) density[i] = 0.f;

    // Compute the voxel size
    dx = bbox.Width() / float(nx);
    dy = bbox.Height() / float(ny);
    dz = bbox.Depth() / float(nz);
}


void VSDGrid::AddEvent(const VSDEvent *event)
{
    // Sample the events along the grid
    uint64_t vx = nx * (floor(event->p.x - bbox.pMin.x) /
                   (bbox.pMax.x - bbox.pMin.x));
    uint64_t vy = ny * (floor(event->p.y - bbox.pMin.y) /
                   (bbox.pMax.y - bbox.pMin.y));
    uint64_t vz = nz * (floor(event->p.z - bbox.pMin.z) /
                   (bbox.pMax.z - bbox.pMin.z));
    uint64_t voxelIndex(vx + nx * vy + nx * ny * vz);
    density[voxelIndex] += event->power;
}


void VSDGrid::Shift(const float &x, const float &y, const float &z) {
    bbox.pMin.x -= x;
    bbox.pMax.x -= x;
    bbox.pMin.y -= y;
    bbox.pMax.y -= y;
    bbox.pMin.z -= z;
    bbox.pMax.z -= z;
}


void VSDGrid::GeneratePBRTConfiguration(const string &outputDir,
        const string &prefix, const string &simulationMethod,
        const int &gridBaseResolution, const int &sensorBaseResolution,
        const Point &sensorPosition, const Vector &sensorDimensions,
        const Vector &spriteTranslation) {
    // Adjust configuration data
    float eyeX, eyeY, eyeZ, pX, pY, pZ, normX, normY, normZ;
    float minX, minY, maxX, maxY;
    float xResolution, yResolution;

    /// LookAt()
    // Eye
    eyeX = 0.f; eyeY = sensorPosition.y; eyeZ = 0.f;
    // Origin
    pX = 0.f; pY = 0.f; pZ = 0.f;
    // Normal
    normX = 0.f; normY = 0.f; normZ = -1.f;

    // Compute the sensor resolution
    float largest = sensorDimensions.x;
    if(sensorDimensions.z > largest) largest = sensorDimensions.z;
    xResolution = sensorBaseResolution * (sensorDimensions.x / largest);
    yResolution = sensorBaseResolution * (sensorDimensions.z / largest);

    // Update the simulation method
    std::string integrator;
    if(simulationMethod == string("backward-direct-grid"))
        integrator = string("vsdbdg");
    else if (simulationMethod == string("backward-linear-grid"))
        integrator = string("vsdblg");
    else if (simulationMethod == string("backward-scattering-grid"))
        integrator = string("vsdbsg");
    else {
        Severe("The simulation method is not specified %s",
               simulationMethod.c_str());
        exit(EXIT_FAILURE);
    }

    // Adjust the frustum
    minX = -sensorDimensions.x / 2;
    maxX = sensorDimensions.x / 2;
    minY = -sensorDimensions.z / 2;
    maxY = sensorDimensions.z / 2;

    // Update output data paths
    string imageName = outputDir + "/" + prefix + ".exr";
    string volumeFileName = outputDir + "/" + prefix + "_volume.raw";
    string volumePrefix = outputDir + "/" + prefix + "_volume";


//    // Write the volume header
//    string fileName = outputDir + "/" + prefix + ".pbrt";
//    ofstream fileStream;
//    fileStream.open(fileName.c_str());
//    fileStream << "### WARNING: Auto Generated File " << N
//               << "# For further detais, look at gluLookAt()" << N
//               << "# eyeX eyeY eyeZ pX pY pZ  normX normY normZ" << N
//               << "LookAt " << eyeX << " " << eyeY << " " << eyeZ << " "
//               << pX << " " << pY << " " << pZ << " "
//               << normX << " " << normY << " " << normZ << " " << NN
//               << "# Camera " << N
//               << "Camera \"orthographic\" \"float screenwindow\" "
//               << "[" << minX << " " << maxX << " "
//               <<  minY << " " << maxY << "]" << NN
//               << "# Film " << N
//               << "Film \"image\"" << N
//               << "\t\"integer xresolution\" " << "[" << xResolution << "]" << N
//               << "\t\"integer yresolution\" " << "[" << yResolution << "]" << N
//               << "\t\"string filename\" " << "\"" << imageName << "\"" << NN
//               << "# Sampler " << N
//               << "Sampler\"bestcandidate\"" << N
//               << "\t\"integer pixelsamples\" [2]" << N
//               << "\tPixelFilter \"triangle\"" << NN
//               << "# Volume Integrator" << N
//               << "VolumeIntegrator \"" << integrator << "\"" << N
//               << "\t\"float stepsize\" " << "[" << 20 << "]" << NN
//               << "# Scene " << N
//               << "WorldBegin" << N
//               << "# Volume" << N
//               << "AttributeBegin" << N
//               // << "Include \"" << volumeFileName << "\"" << N
//               << "Volume \"vsdgrid\" " << std::endl
//               << "\t\"integer nx\" ["   << nx << "]" << std::endl
//               << "\t\"integer ny\" ["   << ny << "]" << std::endl
//               << "\t\"integer nz\" ["   << nz << "]" << std::endl
//               << "\t\"point p0\" [ " << bbox.pMin.x -spriteTranslation.x << " "
//               << bbox.pMin.y -spriteTranslation.y << " "
//               << bbox.pMin.z -spriteTranslation.z << " ]" << std::endl
//               << "\t\"point p1\" [ "
//               << bbox.pMax.x - spriteTranslation.x << " "
//               << bbox.pMax.y - spriteTranslation.y << " "
//               << bbox.pMax.z - spriteTranslation.z << " ]" << std::endl
//               << "\t\"string format\" \"raw\"" << std::endl
//               << "\t\"string prefix\" \"" << volumePrefix << "\""<< std::endl
//               << "\t\"color sigma_s\" "
//               << "[" << SIGMA_S_CORTEX
//               << " " << SIGMA_S_CORTEX
//               << " " << SIGMA_S_CORTEX << "]" << N
//               << "\t\"color sigma_a\" "
//               << "[" << SIGMA_A_CORTEX
//               << " " << SIGMA_A_CORTEX
//               << " " << SIGMA_A_CORTEX << "]" << N
//               << "\t\"color Le\" "
//               << "[" << 0.0 << " "<< 0.0 << " " << 0.0 << "]" << N
//               << "AttributeEnd" << N
//               << "WorldEnd" << N;
//    fileStream.close();

//    printf("done pbrt \n");

    // Write the volume
    // WritePBRTVolumeFile(outputDir, prefix, spriteTranslation);
    WriteRAWVolumeFile(outputDir, prefix, spriteTranslation);
}

void VSDGrid::WritePBRTVolumeFile(const string &outputDir, const string &prefix,
        const Vector &translation) {
    // Full file name
    string fileName = outputDir + "/" + prefix + "_volume.pbrt";
    ofstream fileStream;
    fileStream.open(fileName.c_str());

    // Write the volume header
    fileStream << "Volume \"vsdgrid\" " << std::endl
               << "\t\"integer nx\" "   << nx << std::endl
               << "\t\"integer ny\" "   << ny << std::endl
               << "\t\"integer nz\" "   << nz << std::endl
               << "\t\"point p0\" [ "
               << bbox.pMin.x -translation.x << " "
               << bbox.pMin.y -translation.y << " "
               << bbox.pMin.z -translation.z << " ]" << std::endl
               << "\t\"point p1\" [ "
               << bbox.pMax.x - translation.x << " "
               << bbox.pMax.y - translation.y << " "
               << bbox.pMax.z - translation.z << " ]" << std::endl
               << "\t\"float density\" [";

    // Write the volume
    uint64_t index = 0;
    for (uint64_t i = 0; i < nx; i++)
        for (uint64_t j = 0; j < ny; j++)
            for (uint64_t k = 0; k < nz; k++)
                fileStream << density[index++] << " ";
    fileStream  << "]" << std::endl;
    fileStream.close();

    printf("DONE: Writing the PBRT volume \n");
}


void VSDGrid::WriteRAWVolumeFile(const string &outputDir, const string &prefix,
        const Vector &translation) {
    string rawFileName = outputDir + "/" + prefix + "_volume.raw";
    ofstream rawFileStream;
    rawFileStream.open(rawFileName.c_str(), ios::binary);

    // Get the maximum value
    float maxValue = 0.0;
    uint64_t index = 0;
    for (uint64_t i = 0; i < nx; i++)
        for (uint64_t j = 0; j < ny; j++)
            for (uint64_t k = 0; k < nz; k++) {
                if (density[index] > maxValue) maxValue = density[index];
                index++;
            }
    // Normalize
    index = 0;
    for (uint64_t i = 0; i < nx; i++)
        for (uint64_t j = 0; j < ny; j++)
            for (uint64_t k = 0; k < nz; k++) {
                rawFileStream << uint8_t(int(255 * (density[index] / maxValue)));
                index++;
            }
    rawFileStream.close();

    // Write the header file
    string hdrFileName = outputDir + "/" + prefix + "_volume.hdr";
    ofstream hdrFileStream;
    hdrFileStream.open(hdrFileName.c_str());
    hdrFileStream << nx << " " << ny << " " << nz   << " ";
    hdrFileStream << bbox.pMin.x - translation.x     << " "
                  << bbox.pMin.y - translation.y     << " "
                  << bbox.pMin.z - translation.z     << " ";
    hdrFileStream << bbox.pMax.x - translation.x    << " "
                  << bbox.pMax.y - translation.y    << " "
                  << bbox.pMax.z - translation.z    << " ";
    hdrFileStream << maxValue                       << " ";
    hdrFileStream << bbox.Width()                   << " "
                  << bbox.Height()                  << " "
                  << bbox.Depth()                   << " ";
    hdrFileStream.close();
}
