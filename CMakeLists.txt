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

list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}" ${CMAKE_MODULE_PATH})

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
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

# glob me some header files
FILE(GLOB ACCELERATORS_HEADERS
    "${PROJECT_SOURCE_DIR}/src/accelerators/*.h")
FILE(GLOB CAMERAS_HEADERS
    "${PROJECT_SOURCE_DIR}/src/cameras/*.h")
FILE(GLOB CORE_HEADERS
    "${PROJECT_SOURCE_DIR}/src/core/*.h" "src/core/*.hh")
FILE(GLOB FILM_HEADERS
    "${PROJECT_SOURCE_DIR}/src/film/*.h")
FILE(GLOB FILTERS_HEADERS
    "${PROJECT_SOURCE_DIR}/src/filters/*.h")
FILE(GLOB INTEGRATORS_HEADERS
    "${PROJECT_SOURCE_DIR}/src/integrators/*.h")
FILE(GLOB LIGHTS_HEADERS
    "${PROJECT_SOURCE_DIR}/src/lights/*.h")
FILE(GLOB MATERIALS_HEADERS
    "${PROJECT_SOURCE_DIR}/src/materials/*.h")
FILE(GLOB RENDERERS_HEADERS
    "${PROJECT_SOURCE_DIR}/src/renderers/*.h")
FILE(GLOB SAMPLERS_HEADERS
    "${PROJECT_SOURCE_DIR}/src/samplers/*.h")
FILE(GLOB SHAPES_HEADERS
    "${PROJECT_SOURCE_DIR}/src/shapes/*.h")
FILE(GLOB TEXTURES_HEADERS
    "${PROJECT_SOURCE_DIR}/src/textures/*.h")
FILE(GLOB VOLUMES_HEADERS
    "${PROJECT_SOURCE_DIR}/src/volumes/*.h")
FILE(GLOB VSD_HEADERS
    "${PROJECT_SOURCE_DIR}/src/vsd/*.h")

FILE(GLOB ACCELERATORS_src
    "${PROJECT_SOURCE_DIR}/src/accelerators/*.cpp")
FILE(GLOB CAMERAS_SOURCES
    "${PROJECT_SOURCE_DIR}/src/cameras/*.cpp")
FILE(GLOB CORE_SOURCES
    "${PROJECT_SOURCE_DIR}/src/core/*.cpp" "src/core/*.c"
    "${PROJECT_SOURCE_DIR}/src/core/*.yy" "src/core/*.ll"
)
FILE(GLOB FILM_SOURCES
    "${PROJECT_SOURCE_DIR}/src/film/*.cpp")
FILE(GLOB FILTERS_SOURCES
    "${PROJECT_SOURCE_DIR}/src/filters/*.cpp")
FILE(GLOB INTEGRATORS_SOURCES
    "${PROJECT_SOURCE_DIR}/src/integrators/*.cpp")
FILE(GLOB LIGHTS_SOURCES
    "${PROJECT_SOURCE_DIR}/src/lights/*.cpp")
FILE(GLOB MATERIALS_SOURCES
    "${PROJECT_SOURCE_DIR}/src/materials/*.cpp")
FILE(GLOB RENDERERS_SOURCES
    "${PROJECT_SOURCE_DIR}/src/renderers/*.cpp")
FILE(GLOB SAMPLERS_SOURCES
    "${PROJECT_SOURCE_DIR}/src/samplers/*.cpp")
FILE(GLOB SHAPES_SOURCES
    "${PROJECT_SOURCE_DIR}/src/shapes/*.cpp")
FILE(GLOB TEXTURE_SOURCES
    "${PROJECT_SOURCE_DIR}/src/textures/*.cpp")
FILE(GLOB VOLUME_SOURCES
    "${PROJECT_SOURCE_DIR}/src/volumes/*.cpp")
FILE(GLOB VSD_SOURCES
    "${PROJECT_SOURCE_DIR}/src/vsd/*.cpp")

INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/src/core"
    "${PROJECT_SOURCE_DIR}/src/3rdparty/ilmbase-1.0.2"
    "${PROJECT_SOURCE_DIR}/src/3rdparty/openexr-1.7.0"
    "${PROJECT_SOURCE_DIR}/src/3rdparty/zlib-1.2.5"
)

ADD_DEFINITIONS(-DPBRT_HAS_OPENEXR -DPBRT_PROBES_NONE)

FIND_PACKAGE(BISON REQUIRED)
FIND_PACKAGE(FLEX REQUIRED)
IF(BISON_FOUND AND FLEX_FOUND)
  SET(BisonOutput ${PROJECT_BINARY_DIR}/pbrtparse.cpp)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${BisonOutput} ${PROJECT_BINARY_DIR}/pbrtparse.h
    DEPENDS ${PROJECT_SOURCE_DIR}/src/core/pbrtparse.yy
    COMMAND ${BISON_EXECUTABLE} -d -v -t
            --output=${PROJECT_BINARY_DIR}/pbrtparse.cpp
            ${PROJECT_SOURCE_DIR}/src/core/pbrtparse.yy
    COMMAND ${CMAKE_COMMAND} -E rename ${PROJECT_BINARY_DIR}/pbrtparse.cpp
                                       ${PROJECT_BINARY_DIR}/pbrtparse.cpp
    COMMENT "Generating pbrtparse.cpp and pbrtparse.h"
  )

  SET(FlexOutput ${PROJECT_BINARY_DIR}/pbrtlex.cpp)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${FlexOutput}
    DEPENDS ${PROJECT_BINARY_DIR}/pbrtparse.h
            ${PROJECT_SOURCE_DIR}/src/core/pbrtlex.ll
    COMMAND ${FLEX_EXECUTABLE}
            -o${FlexOutput}
            ${PROJECT_SOURCE_DIR}/src/core/pbrtlex.ll
    COMMENT "Generating pbrtlex.cpp"
  )

  SET (PBRT_YACC_LEX_SOURCE ${BisonOutput} ${FlexOutput})
ELSE()
  SET (PBRT_YACC_LEX_SOURCE
    ${PROJECT_SOURCE_DIR}/src/core/pbrtlex.cpp
    ${PROJECT_SOURCE_DIR}/src/core/pbrtparse.cpp
    )
ENDIF()

SET(PBRT_CORE_SOURCE
    ${PROJECT_SOURCE_DIR}/src/core/api.cpp
    ${PROJECT_SOURCE_DIR}/src/core/api.h
    ${PROJECT_SOURCE_DIR}/src/core/bitarray.cpp
    ${PROJECT_SOURCE_DIR}/src/core/bitarray.h
    ${PROJECT_SOURCE_DIR}/src/core/camera.cpp
    ${PROJECT_SOURCE_DIR}/src/core/camera.h
    ${PROJECT_SOURCE_DIR}/src/core/diffgeom.cpp
    ${PROJECT_SOURCE_DIR}/src/core/diffgeom.h
    ${PROJECT_SOURCE_DIR}/src/core/error.cpp
    ${PROJECT_SOURCE_DIR}/src/core/error.h
    ${PROJECT_SOURCE_DIR}/src/core/fileutil.cpp
    ${PROJECT_SOURCE_DIR}/src/core/fileutil.h
    ${PROJECT_SOURCE_DIR}/src/core/film.cpp
    ${PROJECT_SOURCE_DIR}/src/core/film.h
    ${PROJECT_SOURCE_DIR}/src/core/filter.cpp
    ${PROJECT_SOURCE_DIR}/src/core/filter.h
    ${PROJECT_SOURCE_DIR}/src/core/floatfile.cpp
    ${PROJECT_SOURCE_DIR}/src/core/floatfile.h
    ${PROJECT_SOURCE_DIR}/src/core/geometry.cpp
    ${PROJECT_SOURCE_DIR}/src/core/geometry.h
    ${PROJECT_SOURCE_DIR}/src/core/imageio.cpp
    ${PROJECT_SOURCE_DIR}/src/core/imageio.h
    ${PROJECT_SOURCE_DIR}/src/core/integrator.cpp
    ${PROJECT_SOURCE_DIR}/src/core/integrator.h
    ${PROJECT_SOURCE_DIR}/src/core/intersection.cpp
    ${PROJECT_SOURCE_DIR}/src/core/intersection.h
    ${PROJECT_SOURCE_DIR}/src/core/kdtree.h
    ${PROJECT_SOURCE_DIR}/src/core/light.cpp
    ${PROJECT_SOURCE_DIR}/src/core/light.h
    ${PROJECT_SOURCE_DIR}/src/core/material.cpp
    ${PROJECT_SOURCE_DIR}/src/core/material.h
    ${PROJECT_SOURCE_DIR}/src/core/memory.cpp
    ${PROJECT_SOURCE_DIR}/src/core/memory.h
    ${PROJECT_SOURCE_DIR}/src/core/mipmap.h
    ${PROJECT_SOURCE_DIR}/src/core/montecarlo.cpp
    ${PROJECT_SOURCE_DIR}/src/core/montecarlo.h
    ${PROJECT_SOURCE_DIR}/src/core/octree.h
    ${PROJECT_SOURCE_DIR}/src/core/parallel.cpp
    ${PROJECT_SOURCE_DIR}/src/core/parallel.h
    ${PROJECT_SOURCE_DIR}/src/core/paramset.cpp
    ${PROJECT_SOURCE_DIR}/src/core/paramset.h
    ${PROJECT_SOURCE_DIR}/src/core/parser.cpp
    ${PROJECT_SOURCE_DIR}/src/core/parser.h
    ${PROJECT_SOURCE_DIR}/src/core/pbrt.h
    ${PROJECT_SOURCE_DIR}/src/core/primitive.cpp
    ${PROJECT_SOURCE_DIR}/src/core/primitive.h
    ${PROJECT_SOURCE_DIR}/src/core/probes.cpp
    ${PROJECT_SOURCE_DIR}/src/core/probes.h
    ${PROJECT_SOURCE_DIR}/src/core/progressreporter.cpp
    ${PROJECT_SOURCE_DIR}/src/core/progressreporter.h
    ${PROJECT_SOURCE_DIR}/src/core/quaternion.cpp
    ${PROJECT_SOURCE_DIR}/src/core/quaternion.h
    ${PROJECT_SOURCE_DIR}/src/core/reflection.cpp
    ${PROJECT_SOURCE_DIR}/src/core/reflection.h
    ${PROJECT_SOURCE_DIR}/src/core/renderer.cpp
    ${PROJECT_SOURCE_DIR}/src/core/renderer.h
    ${PROJECT_SOURCE_DIR}/src/core/rng.cpp
    ${PROJECT_SOURCE_DIR}/src/core/rng.h
    ${PROJECT_SOURCE_DIR}/src/core/sampler.cpp
    ${PROJECT_SOURCE_DIR}/src/core/sampler.h
    ${PROJECT_SOURCE_DIR}/src/core/scene.cpp
    ${PROJECT_SOURCE_DIR}/src/core/scene.h
    ${PROJECT_SOURCE_DIR}/src/core/sensor.cpp
    ${PROJECT_SOURCE_DIR}/src/core/sensor.h
    ${PROJECT_SOURCE_DIR}/src/core/shape.cpp
    ${PROJECT_SOURCE_DIR}/src/core/shape.h
    ${PROJECT_SOURCE_DIR}/src/core/sh.cpp
    ${PROJECT_SOURCE_DIR}/src/core/sh.h
    ${PROJECT_SOURCE_DIR}/src/core/shrots.cpp
    ${PROJECT_SOURCE_DIR}/src/core/spectrum.cpp
    ${PROJECT_SOURCE_DIR}/src/core/spectrum.h
    ${PROJECT_SOURCE_DIR}/src/core/stb_image_write.h
    ${PROJECT_SOURCE_DIR}/src/core/stdafx.cpp
    ${PROJECT_SOURCE_DIR}/src/core/stdafx.h
    ${PROJECT_SOURCE_DIR}/src/core/targa.c
    ${PROJECT_SOURCE_DIR}/src/core/targa.h
    ${PROJECT_SOURCE_DIR}/src/core/texture.cpp
    ${PROJECT_SOURCE_DIR}/src/core/texture.h
    ${PROJECT_SOURCE_DIR}/src/core/timer.cpp
    ${PROJECT_SOURCE_DIR}/src/core/timer.h
    ${PROJECT_SOURCE_DIR}/src/core/transform.cpp
    ${PROJECT_SOURCE_DIR}/src/core/transform.h
    ${PROJECT_SOURCE_DIR}/src/core/volume.cpp
    ${PROJECT_SOURCE_DIR}/src/core/volume.h
    ${PROJECT_SOURCE_DIR}/src/core/volumeutil.cpp
    ${PROJECT_SOURCE_DIR}/src/core/volumeutil.h
)

FIND_PACKAGE( OpenMP )
IF( OPENMP_FOUND )
    SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
    SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
ENDIF( )

ADD_LIBRARY(pbrtlibv2
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
TARGET_LINK_LIBRARIES(pbrt pbrtlibv2)

# pbrt2raw
ADD_EXECUTABLE(pbrt2raw "src/tools/pbrt2raw.cpp")
TARGET_LINK_LIBRARIES(pbrt2raw pbrtlibv2)

# volumizesprite
ADD_EXECUTABLE(volumizesprite "src/tools/volumizesprite.cpp")
TARGET_LINK_LIBRARIES(volumizesprite pbrtlibv2)

# spritebounds
ADD_EXECUTABLE(spritebounds "src/tools/spritebounds.cpp")
TARGET_LINK_LIBRARIES(spritebounds pbrtlibv2)

# exravg
ADD_EXECUTABLE(exravg "src/tools/exravg.cpp")
TARGET_LINK_LIBRARIES(exravg pbrtlibv2)

# exrdiff
ADD_EXECUTABLE(exrdiff "src/tools/exrdiff.cpp")
TARGET_LINK_LIBRARIES(exrdiff pbrtlibv2)

