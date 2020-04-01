
/*
    pbrt source code Copyright(c) 1998-2012 Matt Pharr and Greg Humphreys.

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


// integrators/sensor.cpp*
#include "stdafx.h"
#include "integrators/sensor.h"
#include "sensor.h"
#include "scene.h"
#include "paramset.h"
#include "montecarlo.h"

namespace pbrt
{

// SensorIntegrator Method Definitions
void SensorIntegrator::RequestSamples(Sampler *sampler, Sample *sample,
        const Scene *scene) {
    tauSampleOffset = sample->Add1D(1);
    scatterSampleOffset = sample->Add1D(1);
}


Spectrum SensorIntegrator::Transmittance(const Scene *scene,
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


bool SensorIntegrator::SamplePhoton(const Scene *scene, RNG& rng,
                                    Photon &photon, Normal* normal) {
    if (scene->lights.size() > 0) {
        int nLights = scene->lights.size();
        int ln = min(Floor2Int(rng.RandomFloat() * nLights),
                     nLights-1);
        Light *light = scene->lights[ln];

        float lightPdf;
        LightSample ls(rng.RandomFloat(), rng.RandomFloat(), rng.RandomFloat());
        Spectrum Llight = light->Sample_L(scene, ls, rng.RandomFloat(),
                                          rng.RandomFloat(), photon.ray.time,
                                          &photon.ray, normal, &lightPdf);
        photon.L = Llight / lightPdf;
        return true;
    }
    return false;
}


void SensorIntegrator::PhotonRandomWalk(const Scene *scene, Photon& photon,
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
    p = photon.ray.o;

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
                sensor->RecordHit(ray(tHit), Spectrum(1.0));
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


void SensorIntegrator::VolumeRandomWalk(const Scene *scene,
                                        Photon& photon, RNG& rng) {
    // After sampling a photon from the light source, do a Monte Carlo
    // random walk
    VolumeRegion *vr = scene->volumeRegion;
    if (!vr) return;

    Point p, pPrev;
    Vector wo;
    Spectrum Tr(1.0);

    p = photon.ray.o;

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
                sensor->RecordHit(ray(tHit), Tr * photon.L);
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


void SensorIntegrator::Preprocess(const Scene *scene, const Camera *camera,
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
    // #pragma omp parallel for
    for (uint64_t iphoton = 0; iphoton < photonCount; iphoton++) {

        // Compute the percentage of the photons being sent to the propagate in
        // the volume
        const double percentage = (double(iphoton) / double(photonCount)) * 100;
        fprintf(stderr,"\r%f", percentage);

        Photon photon;
        Normal normal;
        if (!SamplePhoton(scene, rng, photon, &normal)) {
            continue;
        }

        // Do a random walk in the volume.
        // VolumeRandomWalk(scene, photon, rng);

         PhotonRandomWalk(scene, photon, rng);
    }

    printf("\nResults \n");
    uint64_t totalHits = 0;
    for (uint64_t isensor = 0; isensor < scene->sensors.size(); isensor++) {
        // Print the final hits
        printf("The sensor %s was hit %lu times \n",
               scene->sensors[isensor]->ReferenceString().c_str(),
               scene->sensors[isensor]->HitCount());

        totalHits += scene->sensors[isensor]->HitCount();

        // Write the film image.
        scene->sensors[isensor]->WriteFilm();
    }

    printf("Total hists %lu \n", totalHits);
}


Spectrum SensorIntegrator::Li(const Scene *scene, const Renderer *renderer,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        Spectrum *T, MemoryArena &arena) const {
    VolumeRegion *vr = scene->volumeRegion;
    float t0, t1;
    if (!vr || !vr->IntersectP(ray, &t0, &t1) || (t1-t0) == 0.f) {
        *T = 1.f;
        return 0.f;
    }
    // Do single scattering volume integration in _vr_
    Spectrum Lv(0.);

    // Prepare for volume integration stepping
    int nSamples = Ceil2Int((t1-t0) / stepSize);
    float step = (t1 - t0) / nSamples;
    Spectrum Tr(1.f);
    Point p = ray(t0), pPrev;
    Vector w = -ray.d;
    t0 += sample->oneD[scatterSampleOffset][0] * step;

    // Compute sample patterns for single scattering samples
    float *lightNum = arena.Alloc<float>(nSamples);
    LDShuffleScrambled1D(1, nSamples, lightNum, rng);
    float *lightComp = arena.Alloc<float>(nSamples);
    LDShuffleScrambled1D(1, nSamples, lightComp, rng);
    float *lightPos = arena.Alloc<float>(2*nSamples);
    LDShuffleScrambled2D(1, nSamples, lightPos, rng);
    uint32_t sampOffset = 0;
    for (int i = 0; i < nSamples; ++i, t0 += step) {
        // Advance to sample at _t0_ and update _T_
        pPrev = p;
        p = ray(t0);
        Ray tauRay(pPrev, p - pPrev, 0.f, 1.f, ray.time, ray.depth);
        Spectrum stepTau = vr->tau(tauRay,
                                   .5f * stepSize, rng.RandomFloat());
        Tr *= Exp(-stepTau);

        // Possibly terminate ray marching if transmittance is small
        if (Tr.y() < 1e-3) {
            const float continueProb = .5f;
            if (rng.RandomFloat() > continueProb) {
                Tr = 0.f;
                break;
            }
            Tr /= continueProb;
        }

        // Compute single-scattering source term at _p_
        Lv += Tr * vr->Lve(p, w, ray.time);
        Spectrum ss = vr->Sigma_s(p, w, ray.time);
        if (!ss.IsBlack() && scene->lights.size() > 0) {

            int nLights = int(scene->lights.size());
            int ln = min(Floor2Int(lightNum[sampOffset] * nLights),
                         nLights-1);
            Light *light = scene->lights[ln];
            // Add contribution of _light_ due to scattering at _p_
            float pdf;
            VisibilityTester vis;
            Vector wo;
            LightSample ls(lightComp[sampOffset], lightPos[2*sampOffset],
                           lightPos[2*sampOffset+1]);
            Spectrum L = light->Sample_L(p, 0.f, ls, ray.time, &wo, &pdf, &vis);
            if (!L.IsBlack() && pdf > 0.f && vis.Unoccluded(scene)) {

                Spectrum Ld = L * vis.Transmittance(scene, renderer, NULL, rng, arena);
                Lv += Tr * ss * vr->p(p, w, -wo, ray.time) * Ld * float(nLights) /
                        pdf;
            }
        }
        ++sampOffset;
    }
    *T = Tr;
    return Lv * step;
}


SensorIntegrator *CreateSensorIntegrator(const ParamSet &params) {
    float stepSize  = params.FindOneFloat("stepsize", 1.f);
    int photonCount = params.FindOneInt("photoncount", 1000);
    return new SensorIntegrator(stepSize, uint64_t(photonCount));
}

}
