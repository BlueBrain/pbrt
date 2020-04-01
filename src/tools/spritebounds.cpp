#include <../vsd/vsdsprite.h>
#include "../vsd/vsdgrid.h"
#include "pbrt.h"
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char** argv)
{
    if(argc < 7) {
        pbrt::Warning("arguments <PSH> <INPUT_DATA_DIRECTORY> <OUTPUT_DATA_DIRECTORY>"
                      "<X_SHIFT = 0> <Y_SHIFT = 0> <Z_SHIFT = 0>");
        return EXIT_SUCCESS;
    }

    // Get the arguments
    string pshFilePrefix(argv[1]);
    string inputDataDirectory(argv[2]);    
    string outputDataDirectory(argv[3]);
    float xShift = atof(argv[4]);
    float yShift = atof(argv[5]);
    float zShift = atof(argv[6]);

    pbrt::Vector shift(xShift, yShift, zShift);

    // Read the VSD sprite
    pbrt::VSDSprite sprite;
    string pshFile = pshFilePrefix + ".psh";
    sprite.Read(inputDataDirectory, pshFile, false, shift);
    std::string original = pshFilePrefix + ".original";
    sprite.PrintBoundingBox(outputDataDirectory, original);

    std::string centered = pshFilePrefix + ".centered";
    sprite.PrintBoundingBox(outputDataDirectory, centered);

    return EXIT_SUCCESS;
}
