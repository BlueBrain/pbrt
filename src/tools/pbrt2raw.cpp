#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <pbrt.h>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <geometry.h>

using namespace std;
using namespace pbrt;

#define HEADER_EXTENSION ".hdr"
#define RAW_EXTENSION ".img"


bool Replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}


void WriteHeader(string prefix, int nx, int ny, int nz, Vector p0, Vector p1) {
    std::string fileName = prefix + std::string( HEADER_EXTENSION );
    std::fstream header;
    header.open(fileName.c_str(), std::ios::out);
    header << nx << " " << ny << " " << nz << std::endl;
    header << p0.x << " " << p0.y << " " << p0.z << std::endl;
    header << p1.x << " " << p1.y << " " << p1.z << std::endl;
    header.close();
}


void WriteVolume(string prefix, int nx, int ny, int nz, int* data,
        Vector p0, Vector p1) {
    WriteHeader( prefix, nx, ny, nz, p0, p1 );
    std::string fileName = prefix + std::string( RAW_EXTENSION );
    std::fstream image;
    image.open( fileName.c_str(), std::ios::out | std::ios::binary );

    int numVoxels = nx * ny * nz;
    for( size_t voxel = 0; voxel < numVoxels; voxel++ ) {
        uint8_t value = data[voxel];
        image << value;
    }
    image.close();
}


int main( int argc, char** argv )
{
    if(argc < 2) {
        printf("Usage: pbrt2raw <INPUT_VOLUME> <OUTPUT_PREFIX>");
        exit(EXIT_SUCCESS);
    }

    string pbrtFile;
    string prefix = "volume";
    if(argc > 2) {
        pbrtFile = argv[1];
        prefix = argv[2];
    }

    // Parse the pbrt file and get its dimensions and the volume data
    int nx = 0, ny = 0, nz = 0;
    Vector p0, p1;
    int* gridData = 0;

    // Open the data file and write the VSD fluorescence events into them
    std::ifstream fileStream(pbrtFile.c_str());
    if (fileStream.is_open()) {
        // Read line by line
        string line;
        while (getline(fileStream, line)) {
            Replace(line, "[", "");
            Replace(line, "]", "");
            Replace(line, "\"", "");
            istringstream stream(line);
            vector<string> tokens;
            copy(istream_iterator<string>(stream),
                 istream_iterator<string>(),
                 back_inserter(tokens));
            if(line.find("nx") != std::string::npos) {
                nx = atoi(tokens[2].c_str());
            } else if(line.find("ny") != std::string::npos) {
                ny = atoi(tokens[2].c_str());
            } else if(line.find("nz") != std::string::npos) {
                nz = atoi(tokens[2].c_str());
            } else if(line.find("p0") != std::string::npos) {
                p0.x = atof(tokens[2].c_str());
                p0.y = atof(tokens[3].c_str());
                p0.z = atof(tokens[4].c_str());
            } else if(line.find("p1") != std::string::npos) {
                p1.x = atof(tokens[2].c_str());
                p1.y = atof(tokens[3].c_str());
                p1.z = atof(tokens[4].c_str());
            } else if(line.find("density") != std::string::npos) {
                gridData = new int[nx*ny*nz];
                #pragma omp parallel for schedule( dynamic, 1 )
                for (int i = 0; i < nx*ny*nz; i++) {
                    if (atof(tokens[2 + i].c_str()) > 0)
                        gridData[i] = 1;
                    else
                        gridData[i] = 0;
                }
            }
        }
        fileStream.close();
    } else {
        Error("The header file cannot be opened");
        return EXIT_SUCCESS;
    }
    fileStream.close();

    // Write raw volume
    WriteVolume(prefix, nx, ny, nz, gridData, p0, p1);

    return EXIT_SUCCESS;
}

