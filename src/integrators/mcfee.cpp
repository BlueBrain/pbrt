
/*
    pbrt source code Copyright(c) 2015 Marwan Abdellah <marwan.abdellah@epfl.ch>

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


// integrators/mcfee.cpp*
#include "stdafx.h"
#include "vsd.h"
#include "scene.h"
#include "paramset.h"
#include "montecarlo.h"
#include "mcfee.h"
#include "core/light.h"
#include "shapes/bead.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <omp.h>
#include <inttypes.h>
#include <cstdio>
#include <cinttypes>


// MCFEE Method Definitions
void MCFEE::RequestSamples(Sampler *sampler, Sample *sample, const Scene *scene) {
    tauSampleOffset = sample->Add1D(1);
    scatterSampleOffset = sample->Add1D(1);
}


Spectrum MCFEE::Transmittance(const Scene *scene, const Renderer *renderer,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        MemoryArena &arena) const {
    if (!scene->volumeRegion) return Spectrum(1.f);
    float step, offset;
    if (sample) {
        step = stepSize;
        offset = sample->oneD[tauSampleOffset][0];
    }
    else {
        step = 4.f * stepSize;
        offset = rng.RandomFloat();
    }
    Spectrum tau = scene->volumeRegion->tau(ray, step, offset);
    return Exp(-tau);
}


bool MCFEE::ExcitationPath(const Scene *scene, Light* fiber, RNG &rng, Point &hitPoint) {
    // Volume region
    VolumeRegion *vr = scene->volumeRegion;

    // Create a light sample
    LightSample ls(rng);

    // Sample the light source
    Ray *ray = new Ray(); Normal *n = new Normal(); float pdf = 0.0;
    Spectrum lightIntensity = fiber->Sample_L(scene, ls, rng.RandomFloat(),
        rng.RandomFloat(), 0, ray, n, &pdf);

    // Photon position, initially from the sampled ray
    Point p =ray->o, pPrev;

    // Phton direction
    Vector wo = ray->d;

    // Start a MC random walk
    while(vr->WorldBound().Inside(p)) {

        // Build a new ray along the new direction.
        Ray ray(p, wo, 0, INFINITY);

        // Save the old point.
        pPrev = p;

        // Get the optical properties of the tissue
        float scatteringCoff = vr->Sigma_s(p, wo, 0.0).y();
        float attenuationCoeff = vr->Sigma_t(p, wo, 0.0).y();
        float scatteringProb = scatteringCoff / attenuationCoeff;

        // printf("%f ", scatteringProb);

        float distancePdf, tDist;
        if (scatteringProb > rng.RandomFloat()) {
            // Sample a distance along the volume
            vr->SampleDistance(ray, &tDist, p, &distancePdf, rng);
        } else {
            // Photon absorption
            break;
        }

        // If the photon hits the bead return true
        for (size_t i = 0; i < scene->beads.size(); i++) {
            if (scene->beads[i]->IntersectP(ray)) {
                return true;
            }
        }

        // Uniformly sample a direction from the fluorescent event.
        wo = UniformSampleSphere(rng.RandomFloat(), rng.RandomFloat());

        // Get the new interaction point
        p = ray(tDist);
    }

    // If the photon exists the volume area, return false
    return false;
}

void MCFEE::EmissionPath(const Scene *scene, RNG &rng, const Point &hitPoint) {

    // Volume region
    VolumeRegion *vr = scene->volumeRegion;

    // Photon positions
    Point p = hitPoint, pPrev;

    // Photon direction
    Vector wo;

    while(vr->WorldBound().Inside(p)) {
        // Uniformly sample a direction from the fluorescent event.
        wo = UniformSampleSphere(rng.RandomFloat(), rng.RandomFloat());

        // Build a new ray along the new direction.
        Ray ray(p, wo, 0, INFINITY);

        // Save the old point.
        pPrev = p;

        // Get the optical properties of the tissue
        float scatteringCoff = vr->Sigma_sf(p, wo, 0.0, 0);
        float attenuationCoeff = vr->Sigma_t(p, wo, 0.0, 0);
        float scatteringProb = scatteringCoff / attenuationCoeff;

        float distancePdf, tDist;
        if (scatteringProb > rng.RandomFloat()) {
            // Sample a distance along the volume
            vr->SampleDistance_f(ray, &tDist, p, &distancePdf, rng);
        } else {
            // Photon absorption
            break;
        }

        for (uint64_t i= 0; i < scene->sensors.size(); i++) {
            float tHit;
            if (scene->sensors[i]->Hit(ray, &tHit, tDist)) {
                scene->sensors[i]->RecordHit(ray(tHit), Spectrum(1.0));
                break;
            }
        }

        // Get the new interaction point
        p = ray(tDist);
    }
}


/**
 * @brief MCFEE::Preprocess
 * This kernel runs a simulation of exciting fluorescent beads embedded in
 * the tissue and receiving the emitted photons by the sensors.
 * The simulation assumes a single sensor.
 * @param scene Scene configuration.
 */
void MCFEE::Preprocess(const Scene *scene, const Camera *, const Renderer *) {

    RNG rng;
    printf("Number of photons use in the simulation [%zu] \n", numberPhotons);

    // If no volume, terminate.
    VolumeRegion *vr = scene->volumeRegion;
    if (!vr) {
        printf("No tissue volume configured! \n");
        exit(EXIT_SUCCESS);
    }

    // If no lights, terminate
    if(scene->lights.size() == 0) {
        printf("No fiber configured! \n");
        exit(EXIT_SUCCESS);
    }

    // If no sensors, terminate
    if(scene->sensors.size() == 0) {
        printf("No sensors configured! \n");
        exit(EXIT_SUCCESS);
    }

    // This is to double check until further notice ...
    size_t progress = 0;
    #pragma omp parallel for
    for (uint64_t i = 0; i < numberPhotons; i++) {
        // OMP
        #pragma omp atomic
        ++progress;

        // Update progress
        if( omp_get_thread_num() == 0 ) {
            double percentage = 100.0 * progress / numberPhotons;
            printf("\r * Running Simulation [%2.2f %%]", percentage);
            fflush(stdout);
        }

        // If the excitation path excites a bead in the scene
        Point hitPoint;
        if (ExcitationPath(scene, scene->lights[0], rng, hitPoint)) {
            // Activate the emission path
            EmissionPath(scene, rng, hitPoint);
        }
    }
    printf("\n");

    uint64 totalHits = 0;
    for (uint64 i = 0; i < scene->sensors.size(); i++) {
        // Print the final hits
        printf("[%s] was hit [%zu] times \n",
               scene->sensors[i]->ReferenceString().c_str(),
               scene->sensors[i]->HitCount());

        // Getting the total hits
        totalHits += scene->sensors[i]->HitCount();

        // Write the sensor data
        scene->sensors[i]->WriteFilm();
        // scene->sensors[i]->WriteRecords();
    }
    printf("Simulation Done!\n");
    exit(EXIT_SUCCESS);
}


Spectrum MCFEE::Li(const Scene *scene, const Renderer *renderer,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        Spectrum *T, MemoryArena &arena) const {
    // Return zero radiance or a black image from the camera !
    return Spectrum(0.);
}


MCFEE *CreateMCFEE(const ParamSet &params) {
    uint64_t numberPhotons = params.FindOneInt("numberphotons", 10000);
    return new MCFEE(numberPhotons);
}
