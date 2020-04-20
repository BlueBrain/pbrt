
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


// integrators/vsdfls.cpp*
#include "stdafx.h"
#include "vsd.h"
#include "scene.h"
#include "paramset.h"
#include "montecarlo.h"
#include "vsdfls.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <omp.h>

namespace pbrt
{

// VSDForwardLinearIntegrator Method Definitions
void VSDForwardLinearIntegrator::RequestSamples(Sampler *sampler, Sample *sample,
                                               const Scene *scene) {
    tauSampleOffset = sample->Add1D(1);
    scatterSampleOffset = sample->Add1D(1);
}


Spectrum VSDForwardLinearIntegrator::Transmittance(const Scene *scene,
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


void VSDForwardLinearIntegrator::Preprocess(const Scene *scene, const Camera *camera,
                               const Renderer *renderer) {
    const uint64_t numberVSDEvents = sprite.GetNumberEvents();

    size_t progress = 0;
#pragma omp parallel for
    for (uint64_t i = 0; i < numberVSDEvents; i++) {
        #pragma omp atomic
        ++progress;

        if( omp_get_thread_num() == 0 ) {
            double percentage = 100.0 * progress / numberVSDEvents;
            printf("\r[%2.2f %%]", percentage);
            fflush(stdout);
        }

        // Initiate the ray from the event towards the sensor
        Point p = sprite.GetEventPosition(i);
        Ray ray(p, Vector(0, 1, 0), 0.f, INFINITY, 0.f, 0.f);

        // Check for photon intersection with the sensor
        float tHit;
        if (scene->sensors[0]->Intersect(ray, &tHit)) {
            scene->sensors[0]->RecordHit(ray(tHit), 1.0);
        }
    }

    uint64_t totalHits = 0;
    for (uint64_t isensor = 0; isensor < scene->sensors.size(); isensor++) {
        // Print the final hits
        printf("\nThe sensor %s was hit [%d] times \n",
               scene->sensors[isensor]->ReferenceString().c_str(),
               scene->sensors[isensor]->HitCount());

        totalHits += scene->sensors[isensor]->HitCount();
        scene->sensors[isensor]->WriteFilm();
    }
    exit(EXIT_SUCCESS);
}


Spectrum VSDForwardLinearIntegrator::Li(const Scene *scene, const Renderer *renderer,
                           const RayDifferential &ray, const Sample *sample, RNG &rng,
                           Spectrum *T, MemoryArena &arena) const {
    // Return zero radiance or a black image from the camera !
    return Spectrum(0.);
}


VSDForwardLinearIntegrator *CreateVSDForwardLinearIntegrator(const ParamSet &params) {
    const std::string vsdDataDirectory =
            params.FindOneString("vsddatadirectory", "");
    const std::string pshFileName = params.FindOneString("pshfile", "");
    Vector shift;
    shift.x = params.FindOneFloat("xshift", 0);
    shift.y = params.FindOneFloat("yshift", 0);
    shift.z = params.FindOneFloat("zshift", 0);
    return new VSDForwardLinearIntegrator(vsdDataDirectory, pshFileName, shift);
}

}
