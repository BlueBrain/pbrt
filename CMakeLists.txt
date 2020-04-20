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
CMAKE_MINIMUM_REQUIRED( VERSION 3.8 )

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
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/accelerators/*.h")
FILE(GLOB CAMERAS_HEADERS
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/cameras/*.h")
FILE(GLOB CORE_HEADERS
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/core/*.h" "src/pbrtv2/core/*.hh")
FILE(GLOB FILM_HEADERS
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/film/*.h")
FILE(GLOB FILTERS_HEADERS
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/filters/*.h")
FILE(GLOB INTEGRATORS_HEADERS
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/integrators/*.h")
FILE(GLOB LIGHTS_HEADERS
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/lights/*.h")
FILE(GLOB MATERIALS_HEADERS
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/materials/*.h")
FILE(GLOB RENDERERS_HEADERS
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/renderers/*.h")
FILE(GLOB SAMPLERS_HEADERS
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/samplers/*.h")
FILE(GLOB SHAPES_HEADERS
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/shapes/*.h")
FILE(GLOB TEXTURES_HEADERS
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/textures/*.h")
FILE(GLOB VOLUMES_HEADERS
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/volumes/*.h")
FILE(GLOB VSD_HEADERS
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/vsd/*.h")

FILE(GLOB ACCELERATORS_src
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/accelerators/*.cpp")
FILE(GLOB CAMERAS_SOURCES
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/cameras/*.cpp")
FILE(GLOB CORE_SOURCES
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/core/*.cpp" "src/pbrtv2/core/*.c"
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/core/*.yy" "src/pbrtv2/core/*.ll"
)
FILE(GLOB FILM_SOURCES
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/film/*.cpp")
FILE(GLOB FILTERS_SOURCES
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/filters/*.cpp")
FILE(GLOB INTEGRATORS_SOURCES
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/integrators/*.cpp")
FILE(GLOB LIGHTS_SOURCES
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/lights/*.cpp")
FILE(GLOB MATERIALS_SOURCES
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/materials/*.cpp")
FILE(GLOB RENDERERS_SOURCES
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/renderers/*.cpp")
FILE(GLOB SAMPLERS_SOURCES
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/samplers/*.cpp")
FILE(GLOB SHAPES_SOURCES
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/shapes/*.cpp")
FILE(GLOB TEXTURE_SOURCES
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/textures/*.cpp")
FILE(GLOB VOLUME_SOURCES
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/volumes/*.cpp")
FILE(GLOB VSD_SOURCES
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/vsd/*.cpp")

INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/src/pbrtv2"
    "${PROJECT_SOURCE_DIR}/src/pbrtv2/core"
    "${PROJECT_SOURCE_DIR}/src/3rdparty/ilmbase-1.0.2"
    "${PROJECT_SOURCE_DIR}/src/3rdparty/openexr-1.7.0"
    "${PROJECT_SOURCE_DIR}/src/3rdparty/zlib-1.2.5"
)

ADD_DEFINITIONS(-DPBRT_HAS_OPENEXR -DPBRT_PROBES_NONE)

FIND_PACKAGE(BISON)
FIND_PACKAGE(FLEX)
IF(BISON_FOUND AND FLEX_FOUND)
  SET(BisonOutput ${PROJECT_BINARY_DIR}/pbrtparse.cpp)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${BisonOutput} ${PROJECT_BINARY_DIR}/pbrtparse.h
    DEPENDS ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/pbrtparse.yy
    COMMAND ${BISON_EXECUTABLE} -d -v -t
            --output=${PROJECT_BINARY_DIR}/pbrtparse.cpp
            ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/pbrtparse.yy
    COMMAND ${CMAKE_COMMAND} -E rename ${PROJECT_BINARY_DIR}/pbrtparse.cpp
                                       ${PROJECT_BINARY_DIR}/pbrtparse.cpp
    COMMENT "Generating pbrtparse.cpp and pbrtparse.h"
  )

  SET(FlexOutput ${PROJECT_BINARY_DIR}/pbrtlex.cpp)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${FlexOutput}
    DEPENDS ${PROJECT_BINARY_DIR}/pbrtparse.h
            ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/pbrtlex.ll
    COMMAND ${FLEX_EXECUTABLE}
            -o${FlexOutput}
            ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/pbrtlex.ll
    COMMENT "Generating pbrtlex.cpp"
  )

  SET (PBRT_YACC_LEX_SOURCE ${BisonOutput} ${FlexOutput})
ELSE()
  SET (PBRT_YACC_LEX_SOURCE
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/pbrtlex.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/pbrtparse.cpp
    )
ENDIF()

SET(PBRT_CORE_SOURCE
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/api.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/api.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/bitarray.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/bitarray.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/camera.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/camera.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/diffgeom.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/diffgeom.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/error.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/error.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/fileutil.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/fileutil.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/film.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/film.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/filter.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/filter.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/floatfile.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/floatfile.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/geometry.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/geometry.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/imageio.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/imageio.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/integrator.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/integrator.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/intersection.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/intersection.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/kdtree.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/light.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/light.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/material.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/material.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/memory.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/memory.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/mipmap.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/montecarlo.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/montecarlo.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/octree.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/parallel.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/parallel.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/paramset.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/paramset.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/parser.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/parser.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/pbrt.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/primitive.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/primitive.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/probes.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/probes.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/progressreporter.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/progressreporter.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/quaternion.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/quaternion.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/reflection.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/reflection.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/renderer.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/renderer.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/rng.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/rng.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/sampler.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/sampler.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/scene.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/scene.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/sensor.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/sensor.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/shape.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/shape.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/sh.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/sh.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/shrots.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/spectrum.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/spectrum.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/stb_image_write.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/stdafx.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/stdafx.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/targa.c
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/targa.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/texture.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/texture.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/timer.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/timer.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/transform.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/transform.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/volume.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/volume.h
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/volumeutil.cpp
    ${PROJECT_SOURCE_DIR}/src/pbrtv2/core/volumeutil.h
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

TARGET_INCLUDE_DIRECTORIES(
    pbrtlibv2 PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/pbrtv2>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/pbrtv2/core>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Executable
ADD_EXECUTABLE(pbrtv2 "src/pbrtv2/main/pbrt.cpp")
TARGET_LINK_LIBRARIES(pbrtv2 pbrtlibv2)

# pbrt2raw
ADD_EXECUTABLE(pbrt2raw "src/pbrtv2/tools/pbrt2raw.cpp")
TARGET_LINK_LIBRARIES(pbrt2raw pbrtlibv2)

# volumizesprite
ADD_EXECUTABLE(volumizesprite "src/pbrtv2/tools/volumizesprite.cpp")
TARGET_LINK_LIBRARIES(volumizesprite pbrtlibv2)

# spritebounds
ADD_EXECUTABLE(spritebounds "src/pbrtv2/tools/spritebounds.cpp")
TARGET_LINK_LIBRARIES(spritebounds pbrtlibv2)

# exravg
ADD_EXECUTABLE(exravg "src/pbrtv2/tools/exravg.cpp")
TARGET_LINK_LIBRARIES(exravg pbrtlibv2)

# exrdiff
ADD_EXECUTABLE(exrdiff "src/pbrtv2/tools/exrdiff.cpp")
TARGET_LINK_LIBRARIES(exrdiff pbrtlibv2)

install(
    TARGETS pbrtlibv2
    EXPORT pbrtv2-config
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

install(
    EXPORT pbrtv2-config
    DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake/pbrtv2)

install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/src/
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include
    FILES_MATCHING PATTERN "*.h")

