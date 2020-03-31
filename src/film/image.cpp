
/*
    pbrt source code Copyright(c) 1998-2012 Matt Pharr and Greg Humphreys.
                                  2012-2016 Marwan Abdellah.

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


// film/image.cpp*
#include "stdafx.h"
#include "film/image.h"
#include "spectrum.h"
#include "parallel.h"
#include "imageio.h"

#include <fstream>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <pthread.h>


void strip(std::string &fullString, const std::string subString)
{
    std::string::size_type i = fullString.find(subString);
    if (i != std::string::npos)
       fullString.erase(i, subString.length());
}


// ImageFilm Method Definitions
ImageFilm::ImageFilm(int xres, int yres, float filmwidth, float filmheight,
        Filter *filt, const float crop[4], const string &fn, bool openWindow,
        bool pixelspectra, bool avgfilmspectrum,
        bool usefilter, int lambdamin, int lambdamax, bool valid)
    : Film(xres, yres) {
    filter = filt;
    memcpy(cropWindow, crop, 4 * sizeof(float));
    fileNamePrefix = fn;
    xPixelStart = Ceil2Int(xResolution * cropWindow[0]);
    xPixelCount = max(1, Ceil2Int(xResolution * cropWindow[1]) - xPixelStart);
    yPixelStart = Ceil2Int(yResolution * cropWindow[2]);
    yPixelCount = max(1, Ceil2Int(yResolution * cropWindow[3]) - yPixelStart);
    filmWidth = filmwidth;
    filmHeight = filmheight;
    filmArea = filmWidth * filmHeight;
    pixelWidth = filmWidth / xres;
    pixelHeight = filmHeight / yres;
    pixelArea = pixelWidth * pixelHeight;
    writePixelsSpectra = pixelspectra;
    writeImageSpectrum = avgfilmspectrum;
    nIlluminatedPixels = 0;
    useFilter = usefilter;
    lambdaMin = lambdamin;
    lambdaMax = lambdamax;
    validate = valid;

    // Allocate film image storage
    pixels = new BlockedArray<Pixel>(xPixelCount, yPixelCount);

    // Precompute filter weight table
#define FILTER_TABLE_SIZE 16
    filterTable = new float[FILTER_TABLE_SIZE * FILTER_TABLE_SIZE];
    float *ftp = filterTable;
    for (int y = 0; y < FILTER_TABLE_SIZE; ++y) {
        float fy = ((float)y + .5f) *
                   filter->yWidth / FILTER_TABLE_SIZE;
        for (int x = 0; x < FILTER_TABLE_SIZE; ++x) {
            float fx = ((float)x + .5f) *
                       filter->xWidth / FILTER_TABLE_SIZE;
            *ftp++ = filter->Evaluate(fx, fy);
        }
    }

    // Possibly open window for image display
    if (openWindow || PbrtOptions.openWindow) {
        Warning("Support for opening image display window not available in this build.");
    }
}


void ImageFilm::AddSample(const CameraSample &sample,
                          const Spectrum &L) {
    // Compute sample's raster extent
    float dimageX = sample.imageX - 0.5f;
    float dimageY = sample.imageY - 0.5f;
    int x0 = Ceil2Int (dimageX - filter->xWidth);
    int x1 = Floor2Int(dimageX + filter->xWidth);
    int y0 = Ceil2Int (dimageY - filter->yWidth);
    int y1 = Floor2Int(dimageY + filter->yWidth);
    x0 = max(x0, xPixelStart);
    x1 = min(x1, xPixelStart + xPixelCount - 1);
    y0 = max(y0, yPixelStart);
    y1 = min(y1, yPixelStart + yPixelCount - 1);
    if ((x1-x0) < 0 || (y1-y0) < 0)
    {
        PBRT_SAMPLE_OUTSIDE_IMAGE_EXTENT(const_cast<CameraSample *>(&sample));
        return;
    }

    // Clone the input spectrum to the film
    Spectrum Lfilter(0.);
    if (useFilter) {
        for (int i = 0; i < nSpectralSamples; i++)
            Lfilter.SetSpectrumAtSample(i, L.GetSpectrumAtSample(i));

        // Apply the filters
        Lfilter.BlockBand(lambdaMin, lambdaMax);
    }

    // Loop over filter support and add sample to pixel arrays
    float xyz[3];
    Spectrum LImage = Spectrum(0.f);
    if (useFilter)
        LImage = Lfilter;
    else
        LImage = L;

    LImage.ToXYZ(xyz);

    // Precompute $x$ and $y$ filter table offsets
    int *ifx = ALLOCA(int, x1 - x0 + 1);
    for (int x = x0; x <= x1; ++x) {
        float fx = fabsf((x - dimageX) *
                         filter->invXWidth * FILTER_TABLE_SIZE);
        ifx[x-x0] = min(Floor2Int(fx), FILTER_TABLE_SIZE-1);
    }
    int *ify = ALLOCA(int, y1 - y0 + 1);
    for (int y = y0; y <= y1; ++y) {
        float fy = fabsf((y - dimageY) *
                         filter->invYWidth * FILTER_TABLE_SIZE);
        ify[y-y0] = min(Floor2Int(fy), FILTER_TABLE_SIZE-1);
    }
    bool syncNeeded = (filter->xWidth > 0.5f || filter->yWidth > 0.5f);
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            // Evaluate filter value at $(x,y)$ pixel
            int offset = ify[y-y0]*FILTER_TABLE_SIZE + ifx[x-x0];
            float filterWt = filterTable[offset];

            // Update pixel values with filtered sample contribution
            Pixel &pixel = (*pixels)(x - xPixelStart, y - yPixelStart);
            if (!syncNeeded) {
                pixel.Lxyz[0] += filterWt * xyz[0];
                pixel.Lxyz[1] += filterWt * xyz[1];
                pixel.Lxyz[2] += filterWt * xyz[2];
                pixel.weightSum += filterWt;

                if (writePixelsSpectra || writeImageSpectrum) {
                    for (int i = 0; i < nSpectralSamples; i++) {
                        // Add the pixel area into the calculations to account
                        // for the CCD size
                        pixel.L.GetRefToSampleValueAtWavelengthIndex(i) +=
            pixelArea * filterWt * LImage.GetSampleValueAtWavelengthIndex(i);
                    }
                }
            }
            else {
                // Safely update _Lxyz_ and _weightSum_ even with concurrency
                AtomicAdd(&pixel.Lxyz[0], filterWt * xyz[0]);
                AtomicAdd(&pixel.Lxyz[1], filterWt * xyz[1]);
                AtomicAdd(&pixel.Lxyz[2], filterWt * xyz[2]);
                AtomicAdd(&pixel.weightSum, filterWt);

                // Add the sample (SampledSpectrum sample) to the pixel
                if (writePixelsSpectra || writeImageSpectrum) {
                    for (int i = 0; i < nSpectralSamples; i++) {
                        // Add the pixel area into the calculations to account
                        // for the CCD size
                        AtomicAdd(&pixel.L.GetRefToSampleValueAtWavelengthIndex(i),
            pixelArea * filterWt * LImage.GetSampleValueAtWavelengthIndex(i));
                    }
                }
            }
        }
    }
}


void ImageFilm::Splat(const CameraSample &sample, const Spectrum &L) {
    if (L.HasNaNs()) {
        Warning("ImageFilm ignoring splatted spectrum with NaN values");
        return;
    }
    float xyz[3];
    L.ToXYZ(xyz);
    int x = Floor2Int(sample.imageX), y = Floor2Int(sample.imageY);
    if (x < xPixelStart || x - xPixelStart >= xPixelCount ||
        y < yPixelStart || y - yPixelStart >= yPixelCount) return;
    Pixel &pixel = (*pixels)(x - xPixelStart, y - yPixelStart);
    AtomicAdd(&pixel.splatXYZ[0], xyz[0]);
    AtomicAdd(&pixel.splatXYZ[1], xyz[1]);
    AtomicAdd(&pixel.splatXYZ[2], xyz[2]);
}


void ImageFilm::GetSampleExtent(int *xstart, int *xend,
                                int *ystart, int *yend) const {
    *xstart = Floor2Int(xPixelStart + 0.5f - filter->xWidth);
    *xend   = Ceil2Int(xPixelStart + 0.5f + xPixelCount +
                       filter->xWidth);

    *ystart = Floor2Int(yPixelStart + 0.5f - filter->yWidth);
    *yend   = Ceil2Int(yPixelStart + 0.5f + yPixelCount +
                       filter->yWidth);
}


void ImageFilm::GetPixelExtent(int *xstart, int *xend,
                               int *ystart, int *yend) const {
    *xstart = xPixelStart;
    *xend   = xPixelStart + xPixelCount;
    *ystart = yPixelStart;
    *yend   = yPixelStart + yPixelCount;
}


void ImageFilm::WriteImage(float splatScale) {
    // Convert image to RGB and compute final pixel values
    int nPix = xPixelCount * yPixelCount;
    //float* recordedEnergy = new float[nPix];
    int offset = 0;
    for (int y = 0; y < yPixelCount; ++y) {
        for (int x = 0; x < xPixelCount; ++x) {
            XYZToRGB((*pixels)(x, y).Lxyz, &_rgb[3*offset]);
            float r = (*pixels)(x, y).Lxyz[0];
            float g = (*pixels)(x, y).Lxyz[1];
            float b = (*pixels)(x, y).Lxyz[2];
            float average = splatScale * (r + g + b) / 3.f;
            //recordedEnergy[offset] = average;
            if(_rgb[3 * offset    ] != 0 &&
               _rgb[3 * offset + 1] != 0 &&
               _rgb[3 * offset + 2] != 0) {
                this->nIlluminatedPixels += 1;
            }

            float weightSum = (*pixels)(x, y).weightSum;
            if (weightSum != 0.f) {
                float invWt = 1.f / weightSum;
                _rgb[3*offset  ] = max(0.f, _rgb[3*offset  ] * invWt);
                _rgb[3*offset+1] = max(0.f, _rgb[3*offset+1] * invWt);
                _rgb[3*offset+2] = max(0.f, _rgb[3*offset+2] * invWt);
            }

            float splatRGB[3];
            XYZToRGB((*pixels)(x, y).splatXYZ, splatRGB);
            _rgb[3*offset  ] += splatScale * splatRGB[0];
            _rgb[3*offset+1] += splatScale * splatRGB[1];
            _rgb[3*offset+2] += splatScale * splatRGB[2];
            ++offset;
        }
    }
/*
    strip(fileNamePrefix, ".exr");
    string imageFile = fileNamePrefix + ".exr";
    ::WriteImage(imageFile, rgb, NULL, xPixelCount, yPixelCount,
                 xResolution, yResolution, xPixelStart, yPixelStart);

    // Write the VSD image
    const string vsdFile =  fileNamePrefix + ".vsd";
    std::fstream stream(vsdFile.c_str(), ios::out);
    for (uint64_t x = 0; x < xResolution; ++x)
        for (uint64_t y = 0; y < yResolution; ++y)
            stream << x << " " << y << " "
                   << recordedEnergy[x + xResolution* y] << endl;
    stream.close();

    // Release temporary image memory
    delete[] rgb;
    delete[] recordedEnergy;

    // Write the validation image
    WriteValidationImage(0.f);
*/
}


void ImageFilm::WriteValidationImage(float splatScale) {
    int nPix = xPixelCount * yPixelCount;
    float *rgbPixel = new float[3*nPix];
    // Compute the average spectrum for the entire image
    Spectrum LImage(0.);
    if (writePixelsSpectra || writeImageSpectrum) {
        FILE* spectrumFile = 0;
        string spectrumFileName = fileNamePrefix + ".spectrum";
        if (writePixelsSpectra) {
            spectrumFile = fopen(spectrumFileName.c_str(), "w");
        }
        int offset = 0;
        for (int y = 0; y < yPixelCount; ++y) {
            for (int x = 0; x < xPixelCount; ++x) {
                Spectrum LPixel = (*pixels)(x, y).L;
                float weightSum = (*pixels)(x, y).weightSum;
                if (weightSum != 0.f) {
                    float invWt = 1.f / weightSum;
                    for (int i = 0; i < nSpectralSamples; i++) {
                        float sampleValue = max(0.0f,
                        LPixel.GetSampleValueAtWavelengthIndex(i) * invWt);
                        LPixel.SetSampleValueAtWavelengthIndex(i, sampleValue);
                        if (writeImageSpectrum)
                            LImage.SetSampleValueAtWavelengthIndex
            (i, LImage.GetSampleValueAtWavelengthIndex(i) + sampleValue);
                    }
                }
                float xyzPixel[3];
                LPixel.ToXYZ(xyzPixel);
                XYZToRGB(xyzPixel, &rgbPixel[3*offset]);

#ifdef DEBUG_CENTRAL_PIXEL_SPECTRUM
                if(x == ((xPixelCount / 2) - 1) && (y == (yPixelCount / 2) - 1)) {
                    printf("[FILM_INFO] : The recorded spectrum at the "
                           "central pixel of the image is ");
                    LPixel.PrintSpectrum_f();
                }
#endif
                if (writePixelsSpectra) {
                    fprintf(spectrumFile, "\n[%d, %d]_", x, y);
                    LPixel.Write(spectrumFile);
                }
                ++offset;
            }
        }
        if (writePixelsSpectra)
            fclose(spectrumFile);
    }

    // Write film info
    FILE* filmInfo;
    strip(fileNamePrefix, ".exr");
    string filmInfoFile = fileNamePrefix + ".film";
    filmInfo = fopen(filmInfoFile.c_str(), "w"); {
        // Total power arriving to the film
        float totalRadiance = LImage.CalculateSPDPower();
        float totalIrradiance = totalRadiance * M_PI;
        float totalPower = totalIrradiance * filmArea;

        // Average power per pixel
        Spectrum LAverage(0.f);
        if (nIlluminatedPixels > 0)
            LAverage = LImage / nIlluminatedPixels;
        float avgRadiance = LAverage.CalculateSPDPower();
        float avgIrradiance = avgRadiance * M_PI;
        float avgPower = avgIrradiance * filmArea;

        fprintf(filmInfo, "Film size %f x %f \n", filmWidth, filmHeight);
        fprintf(filmInfo, "pixel size %e x %e \n", pixelWidth, pixelHeight);
        fprintf(filmInfo, "Number illuminated pixels %d \n", nIlluminatedPixels);
        fprintf(filmInfo, "Total L (Radiance) %e \n", totalRadiance);
        fprintf(filmInfo, "Total E (Irradiance) %e \n", totalIrradiance);
        fprintf(filmInfo, "Total Flux (Power) %e \n", totalPower);
        fprintf(filmInfo, "Avg. L (Radiance) %e \n", avgRadiance);
        fprintf(filmInfo, "Avg. E (Irradiance) %e \n", avgIrradiance);
        fprintf(filmInfo, "Avg. Flux (Power) %e \n", avgPower);

    }
    fclose(filmInfo);

    // Write film spectra
    if(writeImageSpectrum) {
        FILE* imageSpectrumFile;
        string imageSpectrumFileName = fileNamePrefix + ".spectrum";
        imageSpectrumFile = fopen(imageSpectrumFileName.c_str(), "w");
        fprintf(imageSpectrumFile, "\n\nAbsolute detected spectrum\n\n");
        LImage.WriteWavelengthIntensity(imageSpectrumFile);

        // Write the normalized spectrum as well
        fprintf(imageSpectrumFile, "\n\nNormalized spectrum\n\n");
        LImage.Normalize();
        LImage.WriteWavelengthIntensity(imageSpectrumFile);
        fclose(imageSpectrumFile);
    }

    // Write validation image to compare RGB to spectral radiance
    if(validate) {
        string validationFileName = fileNamePrefix + ".validation.exr";
        ::WriteImage(validationFileName, rgbPixel, NULL, xPixelCount, yPixelCount,
                     xResolution, yResolution, xPixelStart, yPixelStart);
    }

    delete[] rgbPixel;
}


void ImageFilm::UpdateDisplay(int x0, int y0, int x1, int y1,
    float splatScale) {
}


ImageFilm *CreateImageFilm(const ParamSet &params, Filter *filter) {
    // Intentionally use FindOneString() rather than FindOneFilename() here
    // so that the rendered image is left in the working directory, rather
    // than the directory the scene file lives in.
    string filename = params.FindOneString("filename", "");
    if (PbrtOptions.imageFile != "") {
        if (filename != "") {
            Warning("Output filename supplied on command line, \"%s\", ignored "
                    "due to filename provided in scene description file, \"%s\".",
                    PbrtOptions.imageFile.c_str(), filename.c_str());
        }
        else
            filename = PbrtOptions.imageFile;
    }
    if (filename == "")
#ifdef PBRT_HAS_OPENEXR
        filename = "pbrt.exr";
#else
        filename = "pbrt.tga";
#endif

    int xres = params.FindOneInt("xresolution", 640);
    int yres = params.FindOneInt("yresolution", 480);
    // Pixel size in um
    float pixelWidth_um = params.FindOneFloat("pixelwidth", 1);
    float pixelHeight_um = params.FindOneFloat("pixelheight", 1);
    if (PbrtOptions.quickRender) xres = max(1, xres / 4);
    if (PbrtOptions.quickRender) yres = max(1, yres / 4);
    bool openwin = params.FindOneBool("display", false);
    float crop[4] = { 0, 1, 0, 1 };
    int cwi;
    const float *cr = params.FindFloat("cropwindow", &cwi);
    if (cr && cwi == 4) {
        crop[0] = Clamp(min(cr[0], cr[1]), 0., 1.);
        crop[1] = Clamp(max(cr[0], cr[1]), 0., 1.);
        crop[2] = Clamp(min(cr[2], cr[3]), 0., 1.);
        crop[3] = Clamp(max(cr[2], cr[3]), 0., 1.);
    }

    bool validate = params.FindOneBool("validate", false);
    bool writeSpectrum = params.FindOneBool("writefilmspectrum", false);
    bool writeAverageSpectrum = params.FindOneBool("writeaveragefilmspectrum", false);
    bool spectralfilter = params.FindOneBool("spectralfilter", false);
    int filterBandMin = params.FindOneInt("filter1bandmin", 400);
    int filterBandMax = params.FindOneInt("filter1bandmax", 700);
    return new ImageFilm(xres, yres, pixelWidth_um, pixelHeight_um,
            filter, crop, filename, openwin, writeSpectrum, writeAverageSpectrum,
            spectralfilter, filterBandMin, filterBandMax, validate);
}
