#!/usr/bin/env bash
# Copyright BBP/EPFL (c) 2015 - 2016
# Author : Marwan Abdellah (marwan.abdellah@epfl.ch)
# This shell script is intended to be used to run the VSDI simulation on a local
# machine with a single command-line call after updating all the command line 
# arguments internally.
################################################################################

################################################################################
### UPDATE THE PARAMETERS HERE ... AND THEN RUN THIS SHELL SCRIPT :)
################################################################################
# The directory where the point sprites are generated
#INPUT_DIRECTORY=/gpfs/bbp.epfl.ch/project/proj3/in_silico_experiments/vsd-signals/input/stimulus0_seed19837_mc2
#INPUT_DIRECTORY=/gpfs/bbp.epfl.ch/project/proj3/in_silico_experiments/vsd-signals/input/random-sample/input
#OUTPUT_DIRECTORY=/gpfs/bbp.epfl.ch/project/proj3/in_silico_experiments/vsd-signals/input/random-sample/output-cluster


#INPUT_DIRECTORY=/gpfs/bbp.epfl.ch/project/proj3/in_silico_experiments/vsd-signals/input/stimulus0_seed19837_mc2
#OUTPUT_DIRECTORY=/gpfs/bbp.epfl.ch/project/proj3/in_silico_experiments/vsd-signals/output/stimulus0_seed19837_mc2-updated

INPUT_DIRECTORY=/gpfs/bbp.cscs.ch/project/proj45/vsd/output/reyes-puerta/seed7/mc2
#OUTPUT_DIRECTORY=/gpfs/bbp.epfl.ch/project/proj3/in_silico_experiments/vsd-signals/output/seed3-fds/
OUTPUT_DIRECTORY=/gpfs/bbp.cscs.ch/project/proj3/in_silico_experiments/vsd-signals/output/seed7-blg/


#/gpfs/bbp.cscs.ch/scratch/gss/bgq/newton/stim-strength-sweep/Ca1p24/stimulus*/seed*/mc2
# INPUT_DIRECTORY=/gpfs/bbp.cscs.ch/scratch/gss/bgq/newton/stim-strength-sweep/Ca1p24/stimulus0/seed421432/mc2

# Simulation directories on Taylor's scratch
# /gpfs/bbp.cscs.ch/scratch/gss/bgq/newton/output
# /gpfs/bbp.cscs.ch/scratch/gss/bgq/newton/output/pulse-train-0/Ca1p24/stimulus0/seed113205/mc2

# The directory where the final results will be produced
# OUTPUT_DIRECTORY=/gpfs/bbp.cscs.ch/project/proj3/resources/vsd/13.03.2017/output-single
# OUTPUT_DIRECTORY=/gpfs/bbp.cscs.ch/project/proj3/in_silico_experiments/vsd_singals/output/Ca1p24_stimulus0_seed421432_mc2
# OUTPUT_DIRECTORY=/gpfs/bbp.epfl.ch/project/proj3/in_silico_experiments/vsd-signals/output/24.05.2017/stimulus0_seed19837_mc2

################################################################################
### Simulation method
### [ direct, linear, scattering ] & [ sprite, grid ] & [ forward, backward ] 
################################################################################
### direct 
# (forward-direct-sprite): a direct projection of the sprites on a sensor
# (backward-direct-grid): a direct back projection from a volume grid to the camera
### linear 
# (forward-linear-sprite): a linear attenuation with beer-lambert law from the 
#                          sprite towards the surface of the sensor 
# (backward-linear-grid): linear attenuation with beer-lambert law to a volume 
#                         grid from the camera
### scattering
# (forward-scattering-sprite): forward scattering with monte carlo random walk 
#                              from the VSD source to the sensor
# (backward-scattering-grid): backward scattering with monte carlo random walk 
#                             from the camera to the VSD source
SIMULATION_METHOD=backward-linear-grid

# A particular time-step to simulate. If you need to simulate the entire time series
# that is found in the input directory, just set this option to 'series' and
# this will generate frames for all the steps in that series.  
PSH_FILE=series
# PSH_FILE=frame1398.0.psh

# The pbrt sensor configuration for sprites 
PBRT_SPRITE_SENSOR_CONFIG=$PWD/configurations/sensor_sprite.input

# The pbrt sensor configuration for volumes 
PBRT_VOLUME_SENSOR_CONFIG=$PWD/configurations/sensor_volume.input

# The pbrt executable
PBRT_EXECUTABLE=$PWD/../build/bin/pbrt

# Sprite volumizer executable 
SRPITE_VOLUMIZER_EXECUTABLE=$PWD/../build/bin/volumizesprite

# Sprite bounds executable 
SRPITE_BOUNDS_EXECUTABLE=$PWD/../build/bin/spritebounds

# A configuration file contains data parameters 
DATA_CONFIG_FILE=/gpfs/bbp.cscs.ch/project/proj3/resources/vsd/30.01.2017/mosaic.config

# Node, can be 'cluster' or 'local'
NODE='cluster'

# The resolution of the volume grid if the *-grid methods are used 
GRID_RESOLUTION=256

# The resolution of the sensor
SENSOR_RESOLUTION=256

################################################################################
echo "##########################################################################"
echo "### Parameters"
echo "##########################################################################"
echo INPUT_DIRECTORY=$INPUT_DIRECTORY
echo OUTPUT_DIRECTORY=$OUTPUT_DIRECTORY
echo SIMULATION_METHOD=$SIMULATION_METHOD
echo PSH_FILE=$PSH_FILE
echo PBRT_SPRITE_SENSOR_CONFIG=$PBRT_SPRITE_SENSOR_CONFIG
echo PBRT_VOLUME_SENSOR_CONFIG=$PBRT_VOLUME_SENSOR_CONFIG
echo PBRT_EXECUTABLE=$PBRT_EXECUTABLE
echo SRPITE_VOLUMIZER_EXECUTABLE=$SRPITE_VOLUMIZER_EXECUTABLE
echo SRPITE_BOUNDS_EXECUTABLE=$SRPITE_BOUNDS_EXECUTABLE
echo DATA_CONFIG_FILE=$DATA_CONFIG_FILE
echo GRID_RESOLUTION=$GRID_RESOLUTION
echo SENSOR_RESOLUTION=$SENSOR_RESOLUTION
echo NODE=$NODE
echo "##########################################################################"
################################################################################

################################################################################
echo -e "\nRUNNING ... run.py \n"

./scripts/execution/run_in_silico_vsdi.py                                       \
    --input-directory=$INPUT_DIRECTORY                                          \
    --output-directory=$OUTPUT_DIRECTORY                                        \
    --simulation-method=$SIMULATION_METHOD                                      \
    --psh-file=$PSH_FILE                                                        \
    --data-config-file=$DATA_CONFIG_FILE                                        \
    --pbrt-sprite-sensor-config=$PBRT_SPRITE_SENSOR_CONFIG                      \
    --pbrt-volume-sensor-config=$PBRT_VOLUME_SENSOR_CONFIG                      \
    --pbrt-executable=$PBRT_EXECUTABLE                                          \
    --volumizer-executable=$SRPITE_VOLUMIZER_EXECUTABLE                         \
    --sprite-bounds-executable=$SRPITE_BOUNDS_EXECUTABLE                        \
    --grid-resolution=$GRID_RESOLUTION                                          \
    --node=$NODE                                                                \
    --sensor-resolution=$SENSOR_RESOLUTION
    echo -e "\nDONE ... run.py\n"
################################################################################
