#include <../vsd/vsdsprite.h>
#include "../vsd/vsdgrid.h"
#include "pbrt.h"
#include <iostream>
#include <string>

using namespace std;
using namespace pbrt;

int main(int argc, char** argv)
{
    printf("Volumizing Sprite \n");
    if(argc < 13) {
        Warning("arguments <PSH> <INPUT_DATA_DIRECTORY> "
                "<OUTPUT_DATA_DIRECTORY> <SIMULATION_METHOD> <GRID_RESOLUTION> "
                "<SENSOR_WIDTH> <SENSOR_HEIGHT> <SENSOR_CENTER_X> <SENSOR_CENTER_Y>"
                "<SENSOR_RESOLUTION> <MOSAIC_WIDTH> <MOSAIC_HEIGHT> <MOSAIC_DEPTH>");
        return EXIT_SUCCESS;
    }

    // Get the arguments
    string pshFilePrefix(argv[1]);
    string inputDataDirectory(argv[2]);
    string outputDataDirectory(argv[3]);
    string simulationMethod(argv[4]);
    int gridBaseResolution = atoi(argv[5]);

    // The dimensions of the sensor
    Point sensorPosition(atof(argv[6]), atoi(argv[7]), atoi(argv[8]));
    Vector sensorDimensions(atof(argv[9]), 0.f, atof(argv[10]));

    // The translation vector required to align the sprite under the sensor
    Vector spriteTranslation(atof(argv[11]), atof(argv[12]), atof(argv[13]));

#ifdef DEBUG
    printf("Sprite Shift: %f %f %f \n",
           spriteTranslation.x, spriteTranslation.y, spriteTranslation.z);
#endif

    // Read the VSD sprite
    VSDSprite sprite;
    string pshFile = pshFilePrefix + ".psh";
    sprite.Read(inputDataDirectory, pshFile, false);
    sprite.PrintBoundingBox();

    // Voxelize the sprite and write the results to PBRT and RAW volumes
    VSDGrid* grid = sprite.Voxelize(gridBaseResolution);

    grid->GeneratePBRTConfiguration(outputDataDirectory,
                                    sprite.GetTimeStep(),
                                    simulationMethod,
                                    gridBaseResolution,
                                    512,
                                    sensorPosition,
                                    sensorDimensions,
                                    spriteTranslation);

    printf("DONE Volumizing \n");
    return EXIT_SUCCESS;
}
