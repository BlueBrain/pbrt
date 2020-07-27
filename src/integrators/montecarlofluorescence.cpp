
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


// integrators/montecarlofluorescence.cpp*
#include "stdafx.h"
#include "vsd.h"
#include "scene.h"
#include "paramset.h"
#include "montecarlo.h"
#include "montecarlofluorescence.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <omp.h>
#include <inttypes.h>
#include <cstdio>
#include <cinttypes>
 

// MonteCarloFluorescenceIntegrator Method Definitions
void MonteCarloFluorescenceIntegrator::RequestSamples(Sampler *sampler, Sample *sample,
                                               const Scene *scene) {
    tauSampleOffset = sample->Add1D(1);
    scatterSampleOffset = sample->Add1D(1);
}


Spectrum MonteCarloFluorescenceIntegrator::Transmittance(const Scene *scene,
                                      const Renderer *renderer, const RayDifferential &ray,
                                      const Sample *sample, RNG &rng, MemoryArena &arena) const {
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


void MonteCarloFluorescenceIntegrator::Preprocess(const Scene *scene, const Camera *camera,
                               const Renderer *renderer) {

    RNG rng;
    printf("Number of photons use in the simulation [%zu] \n", numberPhotons);

    // This random walk assumes the generation of a photon from an emitter
    // within the volume. The photon will be scattered or absorbed randomly
    // based on the optical properties of the medium.

    // If no volume, terminate.
    VolumeRegion *vr = scene->volumeRegion;
    if (!vr) return;

    // Get a reference to the sensor,
    Sensor* sensor = scene->sensors[0];

    // Get a reference to the interface
    Sensor* interface = scene->sensors[1];

    // This is to double check until further notice ...
    size_t progress = 0;
    #pragma omp parallel for
    for (uint64_t i = 0; i < numberPhotons; i++) {
        #pragma omp atomic
        ++progress;

        if( omp_get_thread_num() == 0 ) {
            double percentage = 100.0 * progress / numberPhotons;
            printf("\r * Running Simulation [%2.2f %%]", percentage);
            fflush(stdout);
        }

        // The photon will move between p and pPrev in the direction wo.
        Point p, pPrev;

        // Initially, sample w0 uniformly
        Vector wo = UniformSampleSphere(rng.RandomFloat(), rng.RandomFloat());

        // Initially, p is the origin of the photon.
        p = beadPosition;

        int bounce = 0;
        while(vr->WorldBound().Inside(p)) {

            // Build a new ray along the new direction.
            Ray ray(p, wo, 0, INFINITY);

            // Save the old point.
            pPrev = p;

            // Get the optical properties of the tissue
            float scatteringCoff = vr->Sigma_s(p, wo, 0.0).y();
            float attenuationCoeff = vr->Sigma_t(p, wo, 0.0).y();
            float scatteringProb = scatteringCoff / attenuationCoeff;

            float distancePdf, tDist;
            if (scatteringProb > rng.RandomFloat()) {
                // Sample a distance along the volume
                vr->SampleDistance(ray, &tDist, p, &distancePdf, rng);
            } else {
                // Photon absorption
                break;
            }

            // If the sensor hits the interface
            float tHitInterface;
            if (interface->Hit(ray, &tHitInterface, tDist)) {
                 interface->RecordHitAndAngles(ray(tHitInterface), Spectrum(1.0), ray);
                /// Find the refracted ray
                Ray refractedRay;

                // If the ray is refracted
                if(interface->ComputeRefractedRay(ray, tHitInterface, refractedRay)) {
                    // Hits the sensor
                    float tHitSensor;
                    if (sensor->Intersect(refractedRay, &tHitSensor)){
                        sensor->RecordHitAndAngles(refractedRay(tHitSensor), Spectrum(1.0), refractedRay);
                    }

                    // Escaped
                    break;
                }

                // Otherwise reflected, we don't consider the reflected rays.
                break;
            }

//            float tHit;
//            if (sensor->Hit(ray, &tHit, tDist)) {
//                sensor->RecordHit(ray(tHit), Spectrum(1.0));
//                break;
//            }

            // Get the new interaction point
            p = ray(tDist);

            // Get the new direction based on the HG phase function
            float directionPdf;
            vr->SampleDirection(p, ray.d, wo, &directionPdf, rng);
        }
    }
    printf("\n");

    uint64_t totalHits = 0;
    for (uint64_t isensor = 0; isensor < scene->sensors.size(); isensor++) {
        // Print the final hits
        printf("[%s] was hit [%zu] times \n",
               scene->sensors[isensor]->ReferenceString().c_str(),
               scene->sensors[isensor]->HitCount());

        totalHits += scene->sensors[isensor]->HitCount();
        scene->sensors[isensor]->WriteFilm();
        scene->sensors[isensor]->WriteRecords();
    }


    exit(EXIT_SUCCESS);
}


Spectrum MonteCarloFluorescenceIntegrator::Li(const Scene *scene, const Renderer *renderer,
                           const RayDifferential &ray, const Sample *sample, RNG &rng,
                           Spectrum *T, MemoryArena &arena) const {
    // Return zero radiance or a black image from the camera !
    return Spectrum(0.);
}


MonteCarloFluorescenceIntegrator *CreateMonteCarloFluorescenceIntegrator(const ParamSet &params) {
    Point beadPosition = params.FindOnePoint("beadposition", Point());
    uint64_t numberPhotons = params.FindOneInt("numberphotons", 10000);
    return new MonteCarloFluorescenceIntegrator(beadPosition, numberPhotons);
}
