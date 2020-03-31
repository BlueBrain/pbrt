##############################################################################
#    pbrt source code Copyright(c) 1998-2012 Matt Pharr and Greg Humphreys.
#                                  2012-2016 Marwan Abdellah.
#
#    This file is part of pbrt.
#
#    Redistribution and use in source and binary forms, with or without
#    modification, are permitted provided that the following conditions are
#    met:
#
#    - Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#    - Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
#    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
#    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##############################################################################
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

# This version of pbrt is intended to be used for the Blue Brain Project @ the
# Swiss Federal Institute of Technology Lausanne (EPFL), Switzerland.
# This project is available at https://github.com/BlueBrain/pbrt.
# The project is an extension of the open source library present on github at
# https://github.com/mmp/pbrt-v2.
PROJECT(bbp:pbrt-v2)

set(CMAKE_MODULE_PATH $ENV{CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
include(OpenEXR)

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-conversion-null")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-register")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

    FIND_PROGRAM(XIAR xiar)
    IF(XIAR)
    SET(CMAKE_AR "${XIAR}")
    ENDIF(XIAR)
    MARK_AS_ADVANCED(XIAR)

    FIND_PROGRAM(XILD xild)
    IF(XILD)
    SET(CMAKE_LINKER "${XILD}")
    ENDIF(XILD)
    MARK_AS_ADVANCED(XILD)

    SET(FP_MODEL "precise" CACHE STRING "The floating point model to compile with.")
    SET_PROPERTY(CACHE FP_MODEL PROPERTY STRINGS "precise" "fast=1" "fast=2")

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fp-model ${FP_MODEL}")
ENDIF()

IF(MSVC)
    ADD_DEFINITIONS(/D _CRT_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(/D YY_NO_UNISTD_H)
ENDIF()

FIND_PACKAGE(Threads)

ADD_DEFINITIONS(-DNDEBUG -O2 -m64)

# Output directories
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# glob me some header files
FILE(GLOB ACCELERATORS_HEADERS
    "src/accelerators/*.h")
FILE(GLOB CAMERAS_HEADERS
    "src/cameras/*.h")
FILE(GLOB CORE_HEADERS
    "src/core/*.h" "src/core/*.hh")
FILE(GLOB FILM_HEADERS
    "src/film/*.h")
FILE(GLOB FILTERS_HEADERS
    "src/filters/*.h")
FILE(GLOB INTEGRATORS_HEADERS
    "src/integrators/*.h")
FILE(GLOB LIGHTS_HEADERS
    "src/lights/*.h")
FILE(GLOB MATERIALS_HEADERS
    "src/materials/*.h")
FILE(GLOB RENDERERS_HEADERS
    "src/renderers/*.h")
FILE(GLOB SAMPLERS_HEADERS
    "src/samplers/*.h")
FILE(GLOB SHAPES_HEADERS
    "src/shapes/*.h")
FILE(GLOB TEXTURES_HEADERS
    "src/textures/*.h")
FILE(GLOB VOLUMES_HEADERS
    "src/volumes/*.h")
FILE(GLOB VSD_HEADERS
    "src/vsd/*.h")

FILE(GLOB ACCELERATORS_src
    "src/accelerators/*.cpp")
FILE(GLOB CAMERAS_SOURCES
    "src/cameras/*.cpp")
FILE(GLOB CORE_SOURCES
    "src/core/*.cpp" "src/core/*.c"
    "src/core/*.yy" "src/core/*.ll"
)
FILE(GLOB FILM_SOURCES
    "src/film/*.cpp")
FILE(GLOB FILTERS_SOURCES
    "src/filters/*.cpp")
FILE(GLOB INTEGRATORS_SOURCES
    "src/integrators/*.cpp")
FILE(GLOB LIGHTS_SOURCES
    "src/lights/*.cpp")
FILE(GLOB MATERIALS_SOURCES
    "src/materials/*.cpp")
FILE(GLOB RENDERERS_SOURCES
    "src/renderers/*.cpp")
FILE(GLOB SAMPLERS_SOURCES
    "src/samplers/*.cpp")
FILE(GLOB SHAPES_SOURCES
    "src/shapes/*.cpp")
FILE(GLOB TEXTURE_SOURCES
    "src/textures/*.cpp")
FILE(GLOB VOLUME_SOURCES
    "src/volumes/*.cpp")
FILE(GLOB VSD_SOURCES
    "src/vsd/*.cpp")

INCLUDE_DIRECTORIES(
    "src"
    "src/core"
    "src/3rdparty/ilmbase-1.0.2"
    "src/3rdparty/openexr-1.7.0"
    "src/3rdparty/zlib-1.2.5"
)

ADD_DEFINITIONS(-DPBRT_HAS_OPENEXR -DPBRT_PROBES_NONE)

FIND_PACKAGE(BISON REQUIRED)
FIND_PACKAGE(FLEX REQUIRED)
IF(BISON_FOUND AND FLEX_FOUND)
  SET(BisonOutput ${CMAKE_BINARY_DIR}/pbrtparse.cpp)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${BisonOutput} ${CMAKE_BINARY_DIR}/pbrtparse.h
    DEPENDS ${CMAKE_SOURCE_DIR}/src/core/pbrtparse.yy
    COMMAND ${BISON_EXECUTABLE} -d -v -t
            --output=${CMAKE_BINARY_DIR}/pbrtparse.cpp
            ${CMAKE_SOURCE_DIR}/src/core/pbrtparse.yy
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_BINARY_DIR}/pbrtparse.cpp
                                       ${CMAKE_BINARY_DIR}/pbrtparse.cpp
    COMMENT "Generating pbrtparse.cpp and pbrtparse.h"
  )

  SET(FlexOutput ${CMAKE_BINARY_DIR}/pbrtlex.cpp)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${FlexOutput}
    DEPENDS ${CMAKE_BINARY_DIR}/pbrtparse.h
            ${CMAKE_SOURCE_DIR}/src/core/pbrtlex.ll
    COMMAND ${FLEX_EXECUTABLE}
            -o${FlexOutput}
            ${CMAKE_SOURCE_DIR}/src/core/pbrtlex.ll
    COMMENT "Generating pbrtlex.cpp"
  )

  SET (PBRT_YACC_LEX_SOURCE ${BisonOutput} ${FlexOutput})
ELSE()
  SET (PBRT_YACC_LEX_SOURCE
    src/core/pbrtlex.cpp
    src/core/pbrtparse.cpp
    )
ENDIF()

SET(PBRT_CORE_SOURCE
    src/core/api.cpp
    src/core/api.h
    src/core/bitarray.cpp
    src/core/bitarray.h
    src/core/camera.cpp
    src/core/camera.h
    src/core/diffgeom.cpp
    src/core/diffgeom.h
    src/core/error.cpp
    src/core/error.h
    src/core/fileutil.cpp
    src/core/fileutil.h
    src/core/film.cpp
    src/core/film.h
    src/core/filter.cpp
    src/core/filter.h
    src/core/floatfile.cpp
    src/core/floatfile.h
    src/core/geometry.cpp
    src/core/geometry.h
    src/core/imageio.cpp
    src/core/imageio.h
    src/core/integrator.cpp
    src/core/integrator.h
    src/core/intersection.cpp
    src/core/intersection.h
    src/core/kdtree.h
    src/core/light.cpp
    src/core/light.h
    src/core/material.cpp
    src/core/material.h
    src/core/memory.cpp
    src/core/memory.h
    src/core/mipmap.h
    src/core/montecarlo.cpp
    src/core/montecarlo.h
    src/core/octree.h
    src/core/parallel.cpp
    src/core/parallel.h
    src/core/paramset.cpp
    src/core/paramset.h
    src/core/parser.cpp
    src/core/parser.h
    src/core/pbrt.h
    src/core/primitive.cpp
    src/core/primitive.h
    src/core/probes.cpp
    src/core/probes.h
    src/core/progressreporter.cpp
    src/core/progressreporter.h
    src/core/quaternion.cpp
    src/core/quaternion.h
    src/core/reflection.cpp
    src/core/reflection.h
    src/core/renderer.cpp
    src/core/renderer.h
    src/core/rng.cpp
    src/core/rng.h
    src/core/sampler.cpp
    src/core/sampler.h
    src/core/scene.cpp
    src/core/scene.h
    src/core/sensor.cpp
    src/core/sensor.h
    src/core/shape.cpp
    src/core/shape.h
    src/core/sh.cpp
    src/core/sh.h
    src/core/shrots.cpp
    src/core/spectrum.cpp
    src/core/spectrum.h
    src/core/stb_image_write.h
    src/core/stdafx.cpp
    src/core/stdafx.h
    src/core/targa.c
    src/core/targa.h
    src/core/texture.cpp
    src/core/texture.h
    src/core/timer.cpp
    src/core/timer.h
    src/core/transform.cpp
    src/core/transform.h
    src/core/volume.cpp
    src/core/volume.h
    src/core/volumeutil.cpp
    src/core/volumeutil.h
)

FIND_PACKAGE( OpenMP )
IF( OPENMP_FOUND )
    SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
    SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
ENDIF( )

ADD_LIBRARY(pbrtlib
    ${PBRT_CORE_SOURCE}
    ${PBRT_YACC_LEX_SOURCE}
    ${tiff_hdr} ${tiff_src}
    ${ACCELERATORS_HEADERS} ${ACCELERATORS_src}
    ${CAMERAS_HEADERS} ${CAMERAS_SOURCES}
    ${FILM_HEADERS} ${FILM_SOURCES}
    ${FILTERS_HEADERS} ${FILTERS_SOURCES}
    ${INTEGRATORS_HEADERS} ${INTEGRATORS_SOURCES}
    ${LIGHTS_HEADERS} ${LIGHTS_SOURCES}
    ${MATERIALS_HEADERS} ${MATERIALS_SOURCES}
    ${RENDERERS_HEADERS} ${RENDERERS_SOURCES}
    ${SAMPLERS_HEADERS} ${SAMPLERS_SOURCES}
    ${SHAPES_HEADERS} ${SHAPES_SOURCES}
    ${TEXTURES_HEADERS} ${TEXTURE_SOURCES}
    ${VOLUMES_HEADERS} ${VOLUME_SOURCES}
    ${VSD_HEADERS} ${VSD_SOURCES}
)

# Executable
ADD_EXECUTABLE(pbrt "src/main/pbrt.cpp")
TARGET_LINK_LIBRARIES(pbrt pbrtlib)

# pbrt2raw
ADD_EXECUTABLE(pbrt2raw "src/tools/pbrt2raw.cpp")
TARGET_LINK_LIBRARIES(pbrt2raw pbrtlib)

# volumizesprite
ADD_EXECUTABLE(volumizesprite "src/tools/volumizesprite.cpp")
TARGET_LINK_LIBRARIES(volumizesprite pbrtlib)

# spritebounds
ADD_EXECUTABLE(spritebounds "src/tools/spritebounds.cpp")
TARGET_LINK_LIBRARIES(spritebounds pbrtlib)

# exravg
ADD_EXECUTABLE(exravg "src/tools/exravg.cpp")
TARGET_LINK_LIBRARIES(exravg pbrtlib)

# exrdiff
ADD_EXECUTABLE(exrdiff "src/tools/exrdiff.cpp")
TARGET_LINK_LIBRARIES(exrdiff pbrtlib)

