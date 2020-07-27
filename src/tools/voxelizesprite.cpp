#include <../vsd/vsdsprite.h>
#include "../vsd/vsdgrid.h"
#include "pbrt.h"
#include <iostream>
#include <string>

using namespace std;

/**
 * @brief main
 * Read a VSD sprite file and create a volume that voxelizes that space and
 * contains the aggregate of the point sprite data.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char** argv)
{
    printf("Voxelizing Sprite \n");
    if(argc < 5) {
        Warning("arguments "
                "<PSH> "
                "<INPUT_DATA_DIRECTORY> "
                "<OUTPUT_DATA_DIRECTORY> "
                "<GRID_RESOLUTION>");
        return EXIT_SUCCESS;
    }

    // Get the arguments
    string pshFilePrefix(argv[1]);
    string inputDataDirectory(argv[2]);
    string outputDataDirectory(argv[3]);
    int gridBaseResolution = atoi(argv[4]);

    // Read the VSD sprite
    VSDSprite sprite;
    string pshFile = pshFilePrefix + ".psh";
    sprite.Read(inputDataDirectory, pshFile, false);
    sprite.PrintBoundingBox();

    // Voxelize the sprite and write the results to PBRT and RAW volumes
    VSDGrid* grid = sprite.Voxelize(gridBaseResolution);


    grid->WriteRAWVolumeFile(outputDataDirectory, pshFilePrefix, Vector(0.0, 0.0, 0.0));

    printf("DONE Volumizing \n");
    return EXIT_SUCCESS;
}
