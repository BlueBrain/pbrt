
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

// vsd/vsdsprite.cpp*
#include "vsdsprite.h"
#include "stdafx.h"
#include "pbrt.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

namespace pbrt
{

void VSDSprite::ReadHeaderData(const string &filePath) {
    std::vector<std::string> data;
    std::ifstream file(filePath.c_str());
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line))
            data.push_back(line);
        file.close();
    }
    else
        Severe("The specified header [%s] can't be opened \n",
               filePath.c_str());

    for(int i = 0; i < data.size(); i++)
    {
#ifdef DEBUG
        printf("* %s \n", data[i].c_str());
#endif
        if(data[i].find("EventsCount") != std::string::npos)
            eventsCount = ParseIntParameter(data[i]);
        if(data[i].find("XCenter") != std::string::npos)
            center.x = ParseFloatParameter(data[i]);
        if(data[i].find("YCenter") != std::string::npos)
            center.y = ParseFloatParameter(data[i]);
        if(data[i].find("ZCenter") != std::string::npos)
            center.z = ParseFloatParameter(data[i]);
        if(data[i].find("AABBWidth") != std::string::npos)
            dimensions.x = ParseFloatParameter(data[i]);
        if(data[i].find("AABBHeight") != std::string::npos)
            dimensions.y = ParseFloatParameter(data[i]);
        if(data[i].find("AABBDepth") != std::string::npos)
            dimensions.z = ParseFloatParameter(data[i]);
        if(data[i].find("VSDPositionFile") != std::string::npos)
            positionFile = ParseStringParameter(data[i]);
        if(data[i].find("VSDIntensityFile") != std::string::npos)
            intensityFile = ParseStringParameter(data[i]);
        if(data[i].find("TimeStep") != std::string::npos)
            timeStep = ParseStringParameter(data[i]);
    }

    // Update the bounding box based on the data stored in the header file
    bbox.pMin.x = center.x - (dimensions.x / 2.f);
    bbox.pMin.y = center.y - (dimensions.y / 2.f);
    bbox.pMin.z = center.z - (dimensions.z / 2.f);
    bbox.pMax.x = center.x + (dimensions.x / 2.f);
    bbox.pMax.y = center.y + (dimensions.y / 2.f);
    bbox.pMax.z = center.z + (dimensions.z / 2.f);


#ifdef DEBUG
    printf("Sprite: [%s] \n", filePath.c_str());
    printf("\t EventsCount %u \n", eventsCount);
    printf("\t Center [%f %f %f] \n", center.x, center.y, center.z);
    printf("\t Bounding Box [%.5f %.5f %.5f] \n",
           dimensions.x, dimensions.y, dimensions.z);
    printf("\t VSDPositionFile [%s] \n", positionFile.c_str());
    printf("\t VSDIntensityFile [%s] \n", intensityFile.c_str());
#endif
}


void VSDSprite::Read(const string &datadirectory, const string &pshfile,
                     const bool computeBoundingBox,
                     const Vector &translation) {
    // Read the header file to extract the data
    const std::string pshFilePath = datadirectory + "/" + pshfile;
    printf("Header Path \n\t[%s] \n", pshFilePath.c_str());
    ReadHeaderData(pshFilePath);

    // Open the data file and read the fluorescence events
    std::ifstream positionFileStream, intensityFileStream;
    const std::string pspFilePath = datadirectory + "/" + positionFile;
    const std::string psiFilePath = datadirectory + "/" + intensityFile;
    printf("Sources: \n\t[%s]\n\t[%s] \n",
           pspFilePath.c_str(), psiFilePath.c_str());
    positionFileStream.open(pspFilePath.c_str(), std::ios::binary);
    intensityFileStream.open(psiFilePath.c_str(), std::ios::binary);
    if(positionFileStream.is_open() && intensityFileStream.is_open()) {
        for (int i = 0; i < eventsCount; i++) {
            float x, y, z, intensity;
            positionFileStream.read(reinterpret_cast<char*>(&x), sizeof(x));
            positionFileStream.read(reinterpret_cast<char*>(&y), sizeof(y));
            positionFileStream.read(reinterpret_cast<char*>(&z), sizeof(z));
            intensityFileStream.read(reinterpret_cast<char*>(&intensity),
                                     sizeof(intensity));
            events.push_back(VSDEvent(Point(x , y, z) - translation, intensity));
        }
        positionFileStream.close();
        intensityFileStream.close();
    }

    // Double check the number of events in the sprite
    if(events.size() == eventsCount)
        printf("The source data has %d fluorescent events \n",
               int(events.size()));
    else {
        Severe("Wrong sprite count [Given: %d/ Current: %d]\n",
               eventsCount, events.size());
        exit(EXIT_SUCCESS);
    }

    if(computeBoundingBox)
        ComputeBoundingBox();
}

void VSDSprite::Read(const string &datadirectory, const string &pshfile,
                     const Point &pMin, const Point &pMax,
                     const Vector &dimensions, const Vector &shift) {
    // Read the header file to extract the data
    const std::string pshFilePath = datadirectory + "/" + pshfile;

#ifdef DEBUG
    printf("Header Path \n\t[%s] \n", pshFilePath.c_str());
#endif
    ReadHeaderData(pshFilePath);

    // Open the data file and read the fluorescence events
    std::ifstream positionFileStream, intensityFileStream;
    const std::string pspFilePath = datadirectory + "/" + positionFile;
    const std::string psiFilePath = datadirectory + "/" + intensityFile;

    printf("Sources Paths: \n\t[%s]\n\t[%s] \n",
           pspFilePath.c_str(), psiFilePath.c_str());
    positionFileStream.open(pspFilePath.c_str(), std::ios::binary);
    intensityFileStream.open(psiFilePath.c_str(), std::ios::binary);
    if(positionFileStream.is_open() && intensityFileStream.is_open()) {
        for (int i = 0; i < eventsCount; i++) {
            float x, y, z, intensity;
            positionFileStream.read(reinterpret_cast<char*>(&x), sizeof(x));
            positionFileStream.read(reinterpret_cast<char*>(&y), sizeof(y));
            positionFileStream.read(reinterpret_cast<char*>(&z), sizeof(z));
            intensityFileStream.read(reinterpret_cast<char*>(&intensity),
                                     sizeof(intensity));
            events.push_back(VSDEvent(Point(x , y, z) - shift, intensity));
        }
        positionFileStream.close();
        intensityFileStream.close();
    }

    // Update the bounding box from the loaded configuration to avoid computing
    // the bounding box of the sprite again
    bbox.pMin = pMin;
    bbox.pMax = pMax;

    // Double check the number of events in the sprite
    if(events.size() == eventsCount)
        printf("The source data has %d fluorescent events \n",
               int(events.size()));
    else {
        Severe("The number of the events in the sprite is not right \n");
        exit(EXIT_SUCCESS);
    }
}


void VSDSprite::WriteHeaderData(const string &filePath) {
    std::ofstream file(filePath.c_str());
    file.open(filePath.c_str());
    file << "EventsCount=" << eventsCount << std::endl;
    file << "XCenter=" << center.x << std::endl;
    file << "YCenter=" << center.y << std::endl;
    file << "ZCenter=" << center.z << std::endl;
    file << "AABBWidth=" << dimensions.x << std::endl;
    file << "AABBHeight=" << dimensions.y << std::endl;
    file << "AABBDepth=" << dimensions.z << std::endl;
    file << "VSDPositionFile=" << positionFile << std::endl;
    file << "VSDIntensityFile=" << intensityFile << std::endl;
    file << "TimeStep=" << timeStep << std::endl;
    file.close();
}


void VSDSprite::Write(const string &datadirectory, const string &pshfile) {

    // Read the header file to extract the data
    const std::string pshFilePath = datadirectory + "/" + pshfile;
#ifdef DEBUG
    printf("Header Path \n\t[%s] \n", pshFilePath.c_str());
#endif

    WriteHeaderData(pshFilePath);

    // Open the data file and write the VSD fluorescence events into them
    std::ofstream positionFileStream, intensityFileStream;
    const std::string pspFilePath = datadirectory + "/" + positionFile;
    const std::string psiFilePath = datadirectory + "/" + intensityFile;

    positionFileStream.open(pspFilePath.c_str(), std::ios::binary);
    intensityFileStream.open(psiFilePath.c_str(), std::ios::binary);

    if(positionFileStream.is_open() && intensityFileStream.is_open()) {
        for (int i = 0; i < eventsCount; i++) {
            positionFileStream << events[i].p.x;
            positionFileStream << events[i].p.y;
            positionFileStream << events[i].p.z;
            intensityFileStream << events[i].power;
        }
        positionFileStream.close();
        intensityFileStream.close();
    }
}


void VSDSprite::ComputeBoundingBox() {
    Point pMin(INFINITY, INFINITY, INFINITY);
    Point pMax(-INFINITY, -INFINITY, -INFINITY);
    for (int i = 0; i < eventsCount; i++) {
        VSDEvent &event = events[i];

        if (event.p.x < pMin.x) pMin.x = event.p.x;
        if (event.p.y < pMin.y) pMin.y = event.p.y;
        if (event.p.z < pMin.z) pMin.z = event.p.z;

        if (event.p.x > pMax.x) pMax.x = event.p.x;
        if (event.p.y > pMax.y) pMax.y = event.p.y;
        if (event.p.z > pMax.z) pMax.z = event.p.z;
    }

    // Update the bounding box
    bbox.pMin = pMin;
    bbox.pMax = pMax;
}

void VSDSprite::PrintBoundingBox() const
{
    printf("Bounding Box [%s]: %f %f %f %f %f %f \n",
           positionFile.c_str(),
           bbox.pMin.x, bbox.pMin.y, bbox.pMin.z,
           bbox.pMax.x, bbox.pMax.y, bbox.pMax.z);
}


void VSDSprite::PrintBoundingBox(string outputDirectory,
                                 string pshFilePrefix) const
{
    std::string path = outputDirectory + "/" + pshFilePrefix + ".bounds";
    ofstream file;
    file.open(path.c_str());
    file << bbox.pMin.x << "," << bbox.pMin.y << "," << bbox.pMin.z << ","
         << bbox.pMax.x << "," << bbox.pMax.y << "," << bbox.pMax.z;
    file.close();
}


void VSDSprite::Shift(const Vector &shift) {
    // Update the sprite data
    for (int i = 0; i < eventsCount; i++)
        events[i].p -= shift;

    // Update the bounding box
    bbox.pMin = bbox.pMin - shift;
    bbox.pMax = bbox.pMax - shift;
}


void VSDSprite::CenterAtOrigin() {
    // Update the sprite data
    for (int i = 0; i < eventsCount; i++)
        events[i].p -= center;

    // Update the bounding box
    bbox.pMin = bbox.pMin - center;
    bbox.pMax = bbox.pMax - center;
}


int VSDSprite::GetNumberEvents() const {
    return eventsCount;
}


string VSDSprite::GetTimeStep() const {
    return timeStep;
}


float VSDSprite::GetEventIntensity(int i) const {
    return events[i].power;
}


Point VSDSprite::GetEventPosition(int i) const {
    return events[i].p;
}


VSDGrid* VSDSprite::Voxelize(const int resolution) const {
    // Compute the resolution of the grid
    int xresolution, yresolution, zresolution;
    float largest = bbox.Width();
    if(bbox.Height() > largest) largest = bbox.Height();
    if(bbox.Depth() > largest) largest = bbox.Depth();

    xresolution = (bbox.Width() / largest) * resolution;
    yresolution = (bbox.Height() / largest) * resolution;
    zresolution = (bbox.Depth() / largest) * resolution;

#ifndef DEBUG
    printf("VSDGrid resolution [%d %d %d] \n",
           xresolution, yresolution, zresolution);
#endif

    // Create the VSD grid
    VSDGrid* vsdGrid = new VSDGrid(xresolution, yresolution, zresolution, bbox);

    // Sample the events in a Cartesian grid
    for (int i = 0; i < eventsCount; i++)
    {
        vsdGrid->AddEvent(&events[i]);
    }

    return vsdGrid;
}


string ParseStringParameter(const string line) {
    string value;
    stringstream stream(line);
    string token;
    vector<string> tokens;
    char delim = '=';

    while (getline(stream, token, delim))
        tokens.push_back(token);

    value = string(tokens[1]);
    return value;
}


int ParseIntParameter(const string line) {
    string value;
    stringstream stream(line);
    string token;
    vector<string> tokens;
    char delim = '=';

    while (getline(stream, token, delim))
        tokens.push_back(token);

    value = string(tokens[1]);
    return atoi(value.c_str());
}


float ParseFloatParameter(const string line) {
    string value;
    stringstream stream(line);
    string token;
    vector<string> tokens;
    char delim = '=';

    while (getline(stream, token, delim))
        tokens.push_back(token);

    value = string(tokens[1]);
    return atof(value.c_str());
}

}
