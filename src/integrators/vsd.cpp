
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


// integrators/vsdintegrator.cpp*
#include "stdafx.h"
#include "vsd.h"
#include "scene.h"
#include "paramset.h"
#include "montecarlo.h"
#include <sstream>
#include <iostream>
#include <fstream>

// VSDIntegrator Method Definitions
void VSDIntegrator::RequestSamples(Sampler *sampler, Sample *sample,
                                   const Scene *scene) {
    tauSampleOffset = sample->Add1D(1);
    scatterSampleOffset = sample->Add1D(1);
}


Spectrum VSDIntegrator::Transmittance(const Scene *scene,
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


void VSDIntegrator::ReadHeaderData(std::string filePath) {
//    std::ifstream file(filePath.c_str());
//    std::string line;
//    if (file.is_open()) {
//        while (std::getline(file, line))
//        {
//            ParseIntParameter(line, "EventsCount", eventsCount);
//            ParseFloatParameter(line, "XCenter", xCenter);
//            ParseFloatParameter(line, "XCenter", yCenter);
//            ParseFloatParameter(line, "ZCenter", zCenter);
//            ParseFloatParameter(line, "AABBWidth", width);
//            ParseFloatParameter(line, "AABBHeight", height);
//            ParseFloatParameter(line, "AABBDepth", depth);
//            ParseStringParameter(line, "DataFile", spriteFile);
//        }
//        file.close();
//    } else {
//        Error("The specified header [%s] can't be opened \n", filePath.c_str());
//    }
}


void VSDIntegrator::ConstructFluorescenceEventsMap(std::string datadirectory,
                                                   std::string pshfile) {

//    // Read the header file to extract the data
//    const std::string pshFilePath = datadirectory + "/" + pshfile;
//    printf("Path %s \n", pshFilePath.c_str());
//    ReadHeaderData(pshFilePath);

//    // Open the data file and read the fluorescence events
//    std::cout << "Reading the source file \n";
//    std::ifstream file;
//    const std::string pspFilePath = datadirectory + "/" + spriteFile;
//    printf("Source %s \n", pspFilePath.c_str());
//    file.open(pspFilePath.c_str(), std::ios::binary);
//    if(file.is_open())
//    {
//        for (uint64_t i = 0; i < eventsCount; i++)
//        {
//            float x, y, z, power;
//            file.read(reinterpret_cast<char*>(&x), sizeof(x));
//            x -= xCenter;

//            file.read(reinterpret_cast<char*>(&y), sizeof(y));
//            y -= yCenter;

//            file.read(reinterpret_cast<char*>(&z), sizeof(z));
//            z -= zCenter;

//            file.read(reinterpret_cast<char*>(&power), sizeof(power));

//           //  printf("%f %f %f %f \n", x, y, z, power);

//            fluorescenceSources.push_back(FluorescentEvent(Point(x, y, z),
//                                                             Spectrum(1.f)));
//        }

//        file.close();
//    }
//    std::cout << "Done reading the source file \n";
//    std::cout << "The source data has " << fluorescenceSources.size()
//              << " events \n" ;
}


void VSDIntegrator::PhotonRandomWalk(const Scene *scene, FluorescentEvent &event,
                                        RNG& rng) {

    // This random walk assumes the generation of a photon from an emitter
    // within the volume. The photon will be scattered or absorbed randomly
    // based on the optical properties of the medium.

    // If no volume, terminate.
    VolumeRegion *vr = scene->volumeRegion;
    if (!vr) return;

    // The photon will move between p and pPrev in the direction wo.
    Point p, pPrev;
    Vector wo;

    // Initially, p is the origin of the photon.
    p = event.p;

    // To keep track on the photon bounces in the volume.
    int bounce = 0;
    while(vr->WorldBound().Inside(p)) {

        // Uniformly sample a direction from the fluorescent event.
        wo = UniformSampleSphere(rng.RandomFloat(), rng.RandomFloat());

        // Build a new ray along the new direction.
        Ray ray(p, wo, 0, INFINITY);

        // Save the old point.
        pPrev = p;

        // Sample a distance along the volume
        float distancePdf;
        float tDist;
        if(!vr->SampleDistance(ray, &tDist, p, &distancePdf, rng)) {
            Warning("Distance sampling error");
            return;
        }

        // Check for photon escaping the volume
        for (uint64_t sensorId = 0; sensorId < scene->sensors.size(); sensorId++) {
            Sensor* sensor = scene->sensors[sensorId];
            float tHit;
            if (sensor->Hit(ray, &tHit, tDist)) {
                // sensor->RecordHit(ray(tHit), Spectrum(1.0));
                sensor->RecordHitAndAngles(ray(tHit), Spectrum(1.0), ray);
                break;
            }
        }

        // Get the new interaction point
        p = ray(tDist);

        // Find the optical properties at the new interaction point to decide if
        // it is a scattering event or an absorbtion.
        const Spectrum sigma_a = vr->Sigma_a(p, Vector(0, 0, 0), 0);
        const Spectrum sigma_t = vr->Sigma_t(p, Vector(0, 0, 0), 0);
        const float absorbtivity = (sigma_a.y() / sigma_t.y());
        if (absorbtivity > rng.RandomFloat())
            break;
        bounce++;
    }
}


void VSDIntegrator::PhotonPacketRandomWalk(const Scene *scene,
                                        FluorescentEvent &event, RNG& rng) {
    // After sampling a photon from the light source, do a Monte Carlo
    // random walk
    VolumeRegion *vr = scene->volumeRegion;
    if (!vr) return;

    Point p, pPrev;
    Vector wo;
    Spectrum Tr(1.0);

    p = event.p;

    int bounce = 0;
    while(vr->WorldBound().Inside(p)) {

        // Find a new direction
        float directionPdf;

        // Uniformly sample a direction from the fluorescent event.
        wo = UniformSampleSphere(rng.RandomFloat(), rng.RandomFloat());
        directionPdf = 1 / (4 * M_PI);

        Ray ray(p, wo, 0, INFINITY);
        pPrev = p;

        // Sample a distance along the volume
        float distancePdf;
        float tDist;
        if(!vr->SampleDistance(ray, &tDist, p, &distancePdf, rng)) {
            Warning("Distance sampling error");
            return;
        }

        // Check for photon escaping the volume
        for (uint64_t sensorId = 0; sensorId < scene->sensors.size(); sensorId++) {
            Sensor* sensor = scene->sensors[sensorId];
            float tHit;
            if (sensor->Hit(ray, &tHit, tDist)) {
                sensor->RecordHit(ray(tHit), Tr * event.L);
                break;
            }
        }

        p = ray(tDist);

        Ray tauRay(pPrev, p - pPrev, 0.f, 1.f, ray.time, ray.depth);
        Spectrum stepTau = vr->tau(tauRay, .5f * stepSize, rng.RandomFloat());
        Tr *= Exp(-stepTau);

        // Possibly terminate ray marching if transmittance is small
        if (Tr.y() < 1e-3) {
            const float continueProb = .5f;
            if (rng.RandomFloat() > continueProb) {
                Tr = 0.f;
                return;
            }
            Tr /= continueProb;
        }

        // Check if the next event is scattering or not !
        // Find the optical properties at the new interaction point to decide if
        // it is a scattering event or an absorbtion.
        const Spectrum sigma_a = vr->Sigma_a(p, Vector(0, 0, 0), 0);
        const Spectrum sigma_t = vr->Sigma_t(p, Vector(0, 0, 0), 0);
        const float absorbtivity = (sigma_a.y() / sigma_t.y());
        if (absorbtivity > rng.RandomFloat())
            break;
        bounce++;
    }
}


void VSDIntegrator::Preprocess(const Scene *scene, const Camera *camera,
                               const Renderer *renderer) {

    // Get the size of the volume region
    VolumeRegion *vr = scene->volumeRegion;
    if (!vr) return;
    const float sx = vr->WorldBound().pMax.x - vr->WorldBound().pMin.x;
    const float sy = vr->WorldBound().pMax.y - vr->WorldBound().pMin.y;
    const float sz = vr->WorldBound().pMax.z - vr->WorldBound().pMin.z;
    printf("The volume region has the size [%.1f x %.1f x %.1f] \n",
           sx, sy, sz);

    RNG rng;


        // Iterate over all the sources in the volume
        for (uint64_t i = 0; i < fluorescenceSources.size(); i++) {

            // Compute the percentage of the photons being sent to the propagate in
            // the volume
            const double percentage =
                    (double(i) / double(fluorescenceSources.size())) * 100;
            fprintf(stderr,"\r %f ", percentage);

            // Get the source point and do a monte carlo random walk in
            // all the directoins based on the sampling defined
            // in the configuration file    `
            FluorescentEvent source = fluorescenceSources.at(i);
            #pragma omp parallel for
            for (uint64_t j = 0; j < photonsPerEvent; j++) {
                PhotonRandomWalk(scene, source, rng);

            }
        }

        printf("\nResults \n");
        uint64_t totalHits = 0;
        for (uint64_t isensor = 0; isensor < scene->sensors.size(); isensor++) {
            // Print the final hits
            printf("The sensor %s was hit %d times \n",
                   scene->sensors[isensor]->ReferenceString().c_str(),
                   scene->sensors[isensor]->HitCount());

            totalHits += scene->sensors[isensor]->HitCount();

            // Write the film image.
            scene->sensors[isensor]->WriteFilm();
        }

        printf("Total hists %d \n", totalHits);
}


Spectrum VSDIntegrator::Li(const Scene *scene, const Renderer *renderer,
                           const RayDifferential &ray, const Sample *sample, RNG &rng,
                           Spectrum *T, MemoryArena &arena) const {
    // Return zero radiance or a black image
    return Spectrum(0.);
}


VSDIntegrator *CreateVSDIntegrator(const ParamSet &params) {
    const std::string vsdDataDirectory =
            params.FindOneString("vsddatadirectory", "");
    const std::string pshFileName = params.FindOneString("pshfile", "");
    const uint64_t numPhotonsPerEvent = params.FindOneInt("photonsperevent", 1);
    return new VSDIntegrator(vsdDataDirectory, pshFileName, numPhotonsPerEvent);
}


//void ParseStringParameter(const std::string line, const std::string parameter,
//                         std::string& value) {
//    if (line.find(std::string(parameter).append("=")) != std::string::npos) {
//        std::istringstream iss(line);
//        std::string token;
//        int counter = 0;
//        while(std::getline(iss, token, '=')) {
//            if (counter++ == 1) {
//                value = token;
//            }
//        }
//    }
//}


//void ParseIntParameter(const std::string line, const std::string parameter,
//                         u_int64_t &value) {
//    if (line.find(std::string(parameter).append("=")) != std::string::npos) {
//        std::istringstream iss(line);
//        std::string token;
//        int counter = 0;
//        while(std::getline(iss, token, '=')) {
//            if (counter++ == 1) {
//                value = ::atoi(token.c_str());
//            }
//        }
//    }
//}


//void ParseFloatParameter(const std::string line, const std::string parameter,
//                         float& value) {
//    if (line.find(std::string(parameter).append("=")) != std::string::npos) {
//        std::istringstream iss(line);
//        std::string token;
//        int counter = 0;
//        while(std::getline(iss, token, '=')) {
//            if (counter++ == 1) {
//                value = ::atof(token.c_str());
//            }
//        }
//    }
//}
