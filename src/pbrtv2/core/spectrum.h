
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

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PBRT_CORE_SPECTRUM_H
#define PBRT_CORE_SPECTRUM_H

// core/spectrum.h*
#include "pbrt.h"
#include "parallel.h"

namespace pbrt
{

// Spectrum Utility Declarations
static const int sampledLambdaStart = 300;
static const int sampledLambdaEnd = 800;
static const int nSpectralSamples = 500; // 500;
extern bool SpectrumSamplesSorted(const float *lambda, const float *vals, int n);
extern void SortSpectrumSamples(float *lambda, float *vals, int n);
extern float AverageSpectrumSamples(const float *lambda, const float *vals,
    int n, float lambdaStart, float lambdaEnd);
inline void XYZToRGB(const float xyz[3], float rgb[3]) {
    rgb[0] =  3.240479f*xyz[0] - 1.537150f*xyz[1] - 0.498535f*xyz[2];
    rgb[1] = -0.969256f*xyz[0] + 1.875991f*xyz[1] + 0.041556f*xyz[2];
    rgb[2] =  0.055648f*xyz[0] - 0.204043f*xyz[1] + 1.057311f*xyz[2];
}


inline void RGBToXYZ(const float rgb[3], float xyz[3]) {
    xyz[0] = 0.412453f*rgb[0] + 0.357580f*rgb[1] + 0.180423f*rgb[2];
    xyz[1] = 0.212671f*rgb[0] + 0.715160f*rgb[1] + 0.072169f*rgb[2];
    xyz[2] = 0.019334f*rgb[0] + 0.119193f*rgb[1] + 0.950227f*rgb[2];
}


inline void RGBToGray(const float rgb[3], float &gray) {
    gray = 0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2];
}


enum SpectrumType { SPECTRUM_REFLECTANCE, SPECTRUM_ILLUMINANT };
extern void Blackbody(const float *wl, int n, float temp, float *vals);
extern float InterpolateSpectrumSamples(const float *lambda, const float *vals,
                                        int n, float l);

// Spectral Data Declarations
static const int nCIESamples = 471;
extern const float CIE_X[nCIESamples];
extern const float CIE_Y[nCIESamples];
extern const float CIE_Z[nCIESamples];
extern const float CIE_lambda[nCIESamples];
static const float CIE_Y_integral = 106.856895;
static const int nRGB2SpectSamples = 32;
extern const float RGB2SpectLambda[nRGB2SpectSamples];
extern const float RGBRefl2SpectWhite[nRGB2SpectSamples];
extern const float RGBRefl2SpectCyan[nRGB2SpectSamples];
extern const float RGBRefl2SpectMagenta[nRGB2SpectSamples];
extern const float RGBRefl2SpectYellow[nRGB2SpectSamples];
extern const float RGBRefl2SpectRed[nRGB2SpectSamples];
extern const float RGBRefl2SpectGreen[nRGB2SpectSamples];
extern const float RGBRefl2SpectBlue[nRGB2SpectSamples];
extern const float RGBIllum2SpectWhite[nRGB2SpectSamples];
extern const float RGBIllum2SpectCyan[nRGB2SpectSamples];
extern const float RGBIllum2SpectMagenta[nRGB2SpectSamples];
extern const float RGBIllum2SpectYellow[nRGB2SpectSamples];
extern const float RGBIllum2SpectRed[nRGB2SpectSamples];
extern const float RGBIllum2SpectGreen[nRGB2SpectSamples];
extern const float RGBIllum2SpectBlue[nRGB2SpectSamples];

// Spectrum Declarations
template <int nSamples> class CoefficientSpectrum {
public:
    // CoefficientSpectrum Public Methods
    CoefficientSpectrum(float v = 0.f) {
        for (int i = 0; i < nSamples; ++i)
            c[i] = v;
        Assert(!HasNaNs());
    }
#ifdef DEBUG
    CoefficientSpectrum(const CoefficientSpectrum &s) {
        Assert(!s.HasNaNs());
        for (int i = 0; i < nSamples; ++i)
            c[i] = s.c[i];
    }
    
    CoefficientSpectrum &operator=(const CoefficientSpectrum &s) {
        Assert(!s.HasNaNs());
        for (int i = 0; i < nSamples; ++i)
            c[i] = s.c[i];
        return *this;
    }
#endif // DEBUG
    void Print(FILE *f) const {
        fprintf(f, "[ ");
        for (int i = 0; i < nSamples; ++i) {
            fprintf(f, "%f", c[i]);
            if (i != nSamples-1) fprintf(f, ", ");
        }
        fprintf(f, "]");
    }
    CoefficientSpectrum &operator+=(const CoefficientSpectrum &s2) {
        Assert(!s2.HasNaNs());
        for (int i = 0; i < nSamples; ++i)
            c[i] += s2.c[i];
        return *this;
    }
    CoefficientSpectrum operator+(const CoefficientSpectrum &s2) const {
        Assert(!s2.HasNaNs());
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSamples; ++i)
            ret.c[i] += s2.c[i];
        return ret;
    }
    CoefficientSpectrum operator-(const CoefficientSpectrum &s2) const {
        Assert(!s2.HasNaNs());
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSamples; ++i)
            ret.c[i] -= s2.c[i];
        return ret;
    }
    CoefficientSpectrum operator/(const CoefficientSpectrum &s2) const {
        Assert(!s2.HasNaNs());
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSamples; ++i)
            ret.c[i] /= s2.c[i];
        return ret;
    }
    CoefficientSpectrum operator*(const CoefficientSpectrum &sp) const {
        Assert(!sp.HasNaNs());
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSamples; ++i)
            ret.c[i] *= sp.c[i];
        return ret;
    }
    CoefficientSpectrum &operator*=(const CoefficientSpectrum &sp) {
        Assert(!sp.HasNaNs());
        for (int i = 0; i < nSamples; ++i)
            c[i] *= sp.c[i];
        return *this;
    }
    CoefficientSpectrum operator*(float a) const {
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSamples; ++i)
            ret.c[i] *= a;
        Assert(!ret.HasNaNs());
        return ret;
    }
    CoefficientSpectrum &operator*=(float a) {
        for (int i = 0; i < nSamples; ++i)
            c[i] *= a;
        Assert(!HasNaNs());
        return *this;
    }
    friend inline
    CoefficientSpectrum operator*(float a, const CoefficientSpectrum &s) {
        Assert(!isnan(a) && !s.HasNaNs());
        return s * a;
    }
    CoefficientSpectrum operator/(float a) const {
        Assert(!isnan(a));
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSamples; ++i)
            ret.c[i] /= a;
        Assert(!ret.HasNaNs());
        return ret;
    }
    CoefficientSpectrum &operator/=(float a) {
        Assert(!isnan(a));
        for (int i = 0; i < nSamples; ++i)
            c[i] /= a;
        return *this;
    }
    bool operator==(const CoefficientSpectrum &sp) const {
        for (int i = 0; i < nSamples; ++i)
            if (c[i] != sp.c[i]) return false;
        return true;
    }
    bool operator!=(const CoefficientSpectrum &sp) const {
        return !(*this == sp);
    }
    bool IsBlack() const {
        for (int i = 0; i < nSamples; ++i)
            if (c[i] != 0.) return false;
        return true;
    }
    friend CoefficientSpectrum Sqrt(const CoefficientSpectrum &s) {
        CoefficientSpectrum ret;
        for (int i = 0; i < nSamples; ++i)
            ret.c[i] = sqrtf(s.c[i]);
        Assert(!ret.HasNaNs());
        return ret;
    }
    template <int n> friend inline CoefficientSpectrum<n> Pow(const CoefficientSpectrum<n> &s, float e);
    CoefficientSpectrum operator-() const {
        CoefficientSpectrum ret;
        for (int i = 0; i < nSamples; ++i)
            ret.c[i] = -c[i];
        return ret;
    }
    friend CoefficientSpectrum Exp(const CoefficientSpectrum &s) {
        CoefficientSpectrum ret;
        for (int i = 0; i < nSamples; ++i)
            ret.c[i] = expf(s.c[i]);
        Assert(!ret.HasNaNs());
        return ret;
    }
    CoefficientSpectrum Clamp(float low = 0, float high = INFINITY) const {
        CoefficientSpectrum ret;
        for (int i = 0; i < nSamples; ++i)
            ret.c[i] = ::Clamp(c[i], low, high);
        Assert(!ret.HasNaNs());
        return ret;
    }
    bool HasNaNs() const {
        for (int i = 0; i < nSamples; ++i)
            if (isnan(c[i])) return true;
        return false;
    }
    bool Write(FILE *f) const {
        for (int i = 0; i < nSamples; ++i)
            if (fprintf(f, "%f ", c[i]) < 0) return false;
        return true;
    }
    bool WriteSPD(std::string fileName) const {
        std::string file = fileName + ".spd";
        FILE* f = fopen(file.c_str(), "w");
        for (int i = 0; i < nSpectralSamples; ++i)
            fprintf(f, "%d %f\n", (i + sampledLambdaStart), c[i]);
        fclose(f);
        return true;
    }
    /// Writes to a file a pair of wavelength and radiance,
    /// for example, [401 1.2323]
    /// This file will be used to plot spectral distributions.
    bool WriteWavelengthIntensity(FILE *f) const {
        for (int i = 0; i < nSamples; ++i)
        {
            const int lambda = sampledLambdaStart + i;
            if (fprintf(f, "%d %f\n", lambda, c[i]) < 0) return false;
        }
        return true;
    }
    bool Read(FILE *f) {
        for (int i = 0; i < nSamples; ++i)
            if (fscanf(f, "%f ", &c[i]) != 1) return false;
        return true;
    }

protected:
    // CoefficientSpectrum Protected Data
    float c[nSamples];
};


class SampledSpectrum : public CoefficientSpectrum<nSpectralSamples> {
public:
    // SampledSpectrum Public Methods
    SampledSpectrum(float v = 0.f) {
        for (int i = 0; i < nSpectralSamples; ++i) c[i] = v;
    }
    static SampledSpectrum LoadSampledSpectrum(const float *lambda,
                                           const float *v, int n) {
            // Sort samples if unordered, use sorted for returned spectrum
            if (!SpectrumSamplesSorted(lambda, v, n)) {
                vector<float> slambda(&lambda[0], &lambda[n]);
                vector<float> sv(&v[0], &v[n]);
                SortSpectrumSamples(&slambda[0], &sv[0], n);
                return FromSampled(&slambda[0], &sv[0], n);
            }
            SampledSpectrum r;
            for (int i = 0; i < nSpectralSamples; ++i)
                r.c[i] = v[i];
            return r;
        }
    void PrintSpectrum_f() const {
        for (int i = 0; i < nSpectralSamples; ++i)
            printf("%f ", c[i]);
        printf("\n");
    }
    void PrintSpectrum_e() const {
        for (int i = 0; i < nSpectralSamples; ++i)
            printf("%e ", c[i]);
        printf("\n");
    }
    void PrintSpectrum() const {
        for (int i = 0; i < nSpectralSamples; ++i)
            printf("%f(%e) ", c[i], c[i]);
        printf("\n");
    }
    void PrintSpectrumTransposed_f() const {
        for (int i = 0; i < nSpectralSamples; ++i)
            printf("%f \n", c[i]);
        printf("\n");
    }
    void PrintSpectrumTransposed_e() const {
        for (int i = 0; i < nSpectralSamples; ++i)
            printf("%e \n", c[i]);
        printf("\n");
    }
    void PrintSpectrumTransposed() const {
        for (int i = 0; i < nSpectralSamples; ++i)
            printf("%f(%e) \n", c[i], c[i]);
        printf("\n");
    }
    void PrintSpectrumDistribution_f() const {
        for (int i = 0; i < nSpectralSamples; ++i)
            printf("[%d] %f ", i + sampledLambdaStart, c[i]);
        printf("\n");
    }
    void PrintSpectrumDistribution_e() const {
        for (int i = 0; i < nSpectralSamples; ++i)
            printf("[%d] %e ", i + sampledLambdaStart, c[i]);
        printf("\n");
    }
    void PrintSpectrumDistribution() const {
        for (int i = 0; i < nSpectralSamples; ++i)
            printf("[%d] %f(%e) ", i + sampledLambdaStart, c[i], c[i]);
        printf("\n");
    }
    void PrintSpectrumDistributionTransposed_f() const {
        for (int i = 0; i < nSpectralSamples; ++i)
            printf("[%d] %f \n", i + sampledLambdaStart, c[i]);
        printf("\n");
    }
    void PrintSpectrumDistributionTransposed_e() const {
        for (int i = 0; i < nSpectralSamples; ++i)
            printf("[%d] %e \n", i + sampledLambdaStart, c[i]);
        printf("\n");
    }
    void PrintSpectrumDistributionTransposed() const {
        for (int i = 0; i < nSpectralSamples; ++i)
            printf("[%d] %f(%e) \n", i + sampledLambdaStart, c[i], c[i]);
        printf("\n");
    }
    void PrintNonZeroSpectrum_f() const {
        for (int i = 0; i < nSpectralSamples; ++i) {
            if (c[i] > 0) {
                printf("%d, %f ", i + sampledLambdaStart, c[i]);
            }
        }
        printf("\n");
    }
    void PrintNonZeroSpectrum_e() const {
        for (int i = 0; i < nSpectralSamples; ++i) {
            if (c[i] > 0) {
                printf("%d, %e ", i + sampledLambdaStart, c[i]);
            }
        }
        printf("\n");
    }
    void PrintNonZeroSpectrum() const {
        for (int i = 0; i < nSpectralSamples; ++i) {
            if (c[i] > 0) {
                printf("%d, %f(%e) ", i + sampledLambdaStart, c[i], c[i]);
            }
        }
        printf("\n");
    }
    void PrintNonZeroSpectrumTransposed_f() const {
        for (int i = 0; i < nSpectralSamples; ++i) {
            if (c[i] > 0) {
                printf("%d, %f \n", i + sampledLambdaStart, c[i]);
            }
        }
        printf("\n");
    }
    void PrintNonZeroSpectrumTransposed_e() const {
        for (int i = 0; i < nSpectralSamples; ++i) {
            if (c[i] > 0) {
                printf("%d, %e\n", i + sampledLambdaStart, c[i]);
            }
        }
        printf("\n");
    }
    void PrintNonZeroSpectrumTransposed() const {
        for (int i = 0; i < nSpectralSamples; ++i) {
            if (c[i] > 0) {
                printf("%d, %f(%e) \n", i + sampledLambdaStart, c[i], c[i]);
            }
        }
        printf("\n");
    }
    void SetToBlack() {
        for (int i = 0; i < nSpectralSamples; ++i)
            c[i] = 0.0;
    }
    void SetToOne() {
        for (int i = 0; i < nSpectralSamples; ++i)
            c[i] = 1.0;
    }
    void SetToUniformValue(const float value) {
        for (int i = 0; i < nSpectralSamples; ++i)
            c[i] = value;
    }
    void SetSampleValueAtWavelengthIndex(int lambdaIdx, float value) {
        if(lambdaIdx >=0 && lambdaIdx < nSpectralSamples)
            c[lambdaIdx] = value;
        else
            Warning("The specified wavelength index in the function "
                    "SetSampleValueAtWavelengthIndex() is not in the range "
                    "between sampledLambdaStart & sampledLambdaEnd");
    }
    void SetSampleValueAtWavelength(int lambda, float value) {
        if ((lambda < sampledLambdaEnd && lambda > sampledLambdaStart))
            c[lambda - sampledLambdaStart] = value;
        else
            Warning("The specified wavelength in the function "
                    "SetSampleValueAtWavelength() is not in the range "
                    "between sampledLambdaStart & sampledLambdaEnd");
    }
    float GetSampleValueAtWavelength(int lambda) const {
        if ((lambda < sampledLambdaEnd && lambda > sampledLambdaStart))
            return c[lambda - sampledLambdaStart];
        else
            Warning("The specified wavelength in the function "
                    "SetSampleValueAtWavelength() is not in the range "
                    "between sampledLambdaStart & sampledLambdaEnd");
        return 0.0f;
    }
    float GetSampleValueAtWavelengthIndex(int lambdaIdx) const {
        if(lambdaIdx >=0 && lambdaIdx < nSpectralSamples)
            return c[lambdaIdx];
        else
            Warning("The specified wavelength in the function "
                    "SetSampleValueAtWavelength() is not in the range "
                    "between sampledLambdaStart & sampledLambdaEnd");
        return 0.0f;
    }
    float& GetRefToSampleValueAtWavelengthIndex(int lambdaIdx) {
        if(lambdaIdx >=0 && lambdaIdx < nSpectralSamples)
            return c[lambdaIdx];
        else {
            Warning("The specified wavelength in the function "
                    "SetSampleValueAtWavelength() is not in the range "
                    "between sampledLambdaStart & sampledLambdaEnd. "
                    "This function will return c[0]");
            return c[0];
        }
    }
    float CalculateSPDPower (void) const {
        float power = 0;
        for (int i = 0; i < nSpectralSamples; ++i)
            power += c[i];
        return power;
    }
    void NormalizeSPDArea() {
        float power = this->CalculateSPDPower();
        for (int i = 0; i < nSpectralSamples; ++i)
            c[i] /= power;
    }
    void Normalize() {
        float maxValue = 0;
        for (int i = 0; i < nSpectralSamples; ++i) {
            if (c[i] > maxValue) maxValue = c[i];
        }
        for (int i = 0; i < nSpectralSamples; ++i)
            c[i] /= maxValue;
    }
    int GetLaserWavelengthIndex() const {
        int spikeCount = 0;
        int spikeWavelengthIndex;
        for (int i = 0; i < nSpectralSamples; ++i) {
            if (c[i] > 0) {
                spikeCount++;
                spikeWavelengthIndex = i;
            }
        }
        if (spikeCount == 1)
            return (spikeWavelengthIndex);
        else if (spikeCount == 0)
            Warning("The light source power is zero");
        else
            Info("There is more than a single wavelength in your light source");
        return -1;
    }
    int GetLaserWavelength() const {
        return (GetLaserWavelengthIndex() + sampledLambdaStart);
    }
    bool VerifyLaser(int &wavelength, int & wavelengthIndex) const {
        wavelengthIndex = GetLaserWavelengthIndex();
        if (wavelengthIndex > -1 ) {
            wavelength = wavelengthIndex + sampledLambdaStart;
            return true;
        }
        wavelength = -1;
        return false;
    }

    float GetLaserEmissionPower() const {
        return (c[int(GetLaserWavelengthIndex())]);
    }
    float GetLaserEmissionPower(const int lambdaIndex) const {
        return (c[lambdaIndex]);
    }
    SampledSpectrum(const CoefficientSpectrum<nSpectralSamples> &v)
        : CoefficientSpectrum<nSpectralSamples>(v) { }
    static SampledSpectrum FromSampled(const float *lambda,
                                       const float *v, int n) {
        // Sort samples if unordered, use sorted for returned spectrum
        if (!SpectrumSamplesSorted(lambda, v, n)) {
            vector<float> slambda(&lambda[0], &lambda[n]);
            vector<float> sv(&v[0], &v[n]);
            SortSpectrumSamples(&slambda[0], &sv[0], n);
            return FromSampled(&slambda[0], &sv[0], n);
        }
        SampledSpectrum r;
        for (int i = 0; i < nSpectralSamples; ++i) {
            /// Disable the spectrum averaging to enable adding laser lights with
            /// monochromatic wavelength.
            if (nSpectralSamples == (sampledLambdaEnd - sampledLambdaStart + 1)) {
#ifdef DEBUG_SPECTRUM
                Note("Spectrum sampled @ 1nm");
#endif
                r.c[i] = v[i];
            }
            else {
                // Compute average value of given SPD over $i$th sample's range
                float lambda0 = Lerp(float(i) / float(nSpectralSamples),
                                     sampledLambdaStart, sampledLambdaEnd);
                float lambda1 = Lerp(float(i+1) / float(nSpectralSamples),
                                     sampledLambdaStart, sampledLambdaEnd);

                /// Computes the average of the radiance between two wavelengths
                /// NOTE: This function has to be disabled if the spectrum is sampled at 1 nm.
                /// This is becuase it will average a laser spike, or divide its value by half.
                /// _lambda_ is array containing all the wavelengths.
                /// _v_ is the coefficients of spectrum at all wavelengths.
                /// _n_ is number of samples in the spectrum.
                /// _lambda0_ is min wavelength of the line.
                /// _lambda1_ is max wavelength of the line.
                r.c[i] = AverageSpectrumSamples(lambda, v, n, lambda0, lambda1);
            }
        }
        return r;
    }
    static void Init() {
        // Compute XYZ matching functions for _SampledSpectrum_
        for (int i = 0; i < nSpectralSamples; ++i) {
            float wl0 = Lerp(float(i) / float(nSpectralSamples),
                             sampledLambdaStart, sampledLambdaEnd);
            float wl1 = Lerp(float(i+1) / float(nSpectralSamples),
                             sampledLambdaStart, sampledLambdaEnd);
            X.c[i] = AverageSpectrumSamples(CIE_lambda, CIE_X, nCIESamples,
                                            wl0, wl1);
            Y.c[i] = AverageSpectrumSamples(CIE_lambda, CIE_Y, nCIESamples,
                                            wl0, wl1);
            Z.c[i] = AverageSpectrumSamples(CIE_lambda, CIE_Z, nCIESamples,
                                            wl0, wl1);
        }

        // Compute RGB to spectrum functions for _SampledSpectrum_
        for (int i = 0; i < nSpectralSamples; ++i) {
            float wl0 = Lerp(float(i) / float(nSpectralSamples),
                             sampledLambdaStart, sampledLambdaEnd);
            float wl1 = Lerp(float(i+1) / float(nSpectralSamples),
                             sampledLambdaStart, sampledLambdaEnd);
            rgbRefl2SpectWhite.c[i] = AverageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectWhite,
                nRGB2SpectSamples, wl0, wl1);
            rgbRefl2SpectCyan.c[i] = AverageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectCyan,
                nRGB2SpectSamples, wl0, wl1);
            rgbRefl2SpectMagenta.c[i] = AverageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectMagenta,
                nRGB2SpectSamples, wl0, wl1);
            rgbRefl2SpectYellow.c[i] = AverageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectYellow,
                nRGB2SpectSamples, wl0, wl1);
            rgbRefl2SpectRed.c[i] = AverageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectRed,
                nRGB2SpectSamples, wl0, wl1);
            rgbRefl2SpectGreen.c[i] = AverageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectGreen,
                nRGB2SpectSamples, wl0, wl1);
            rgbRefl2SpectBlue.c[i] = AverageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectBlue,
                nRGB2SpectSamples, wl0, wl1);
        
            rgbIllum2SpectWhite.c[i] = AverageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectWhite,
                nRGB2SpectSamples, wl0, wl1);
            rgbIllum2SpectCyan.c[i] = AverageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectCyan,
                nRGB2SpectSamples, wl0, wl1);
            rgbIllum2SpectMagenta.c[i] = AverageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectMagenta,
                nRGB2SpectSamples, wl0, wl1);
            rgbIllum2SpectYellow.c[i] = AverageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectYellow,
                nRGB2SpectSamples, wl0, wl1);
            rgbIllum2SpectRed.c[i] = AverageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectRed,
                nRGB2SpectSamples, wl0, wl1);
            rgbIllum2SpectGreen.c[i] = AverageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectGreen,
                nRGB2SpectSamples, wl0, wl1);
            rgbIllum2SpectBlue.c[i] = AverageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectBlue,
                nRGB2SpectSamples, wl0, wl1);
        }
    }
    void ToXYZ(float xyz[3]) const {
        xyz[0] = xyz[1] = xyz[2] = 0.f;
        for (int i = 0; i < nSpectralSamples; ++i) {
            xyz[0] += X.c[i] * c[i];
            xyz[1] += Y.c[i] * c[i];
            xyz[2] += Z.c[i] * c[i];
        }
        float scale = float(sampledLambdaEnd - sampledLambdaStart) /
            float(CIE_Y_integral * nSpectralSamples);
        xyz[0] *= scale;
        xyz[1] *= scale;
        xyz[2] *= scale;
    }
    float y() const {
        float yy = 0.f;
        for (int i = 0; i < nSpectralSamples; ++i)
            yy += Y.c[i] * c[i];
        return yy * float(sampledLambdaEnd - sampledLambdaStart) /
            float(CIE_Y_integral * nSpectralSamples);
    }
    void ToRGB(float rgb[3]) const {
        float xyz[3];
        ToXYZ(xyz);
        XYZToRGB(xyz, rgb);
    }
    float ToGrayScale() const {
        float rgb[3];
        ToRGB(rgb);
        float gray = 0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2];
        return gray;
    }
    int WavelengthIndex(const int &wl) const {
        Assert(wl >= sampledLambdaStart && wl < sampledLambdaEnd);
        return wl - sampledLambdaStart;
    }
    float Power(const int &wl) const {
        return c[WavelengthIndex(wl)];
    }
    void BlockBand(const int wl1, const int wl2) {
        /// Setting the minimum and the maximum of the filter
        int _wl1, _wl2;
        if (wl1 < sampledLambdaStart)
            _wl1 = sampledLambdaStart;
        else _wl1 = wl1;

        if (wl2 > sampledLambdaEnd)
            _wl2 = sampledLambdaEnd;
        else _wl2 = wl2;

        /// Find the index of the sample
        /// If _wl1 = 400, then wl1_index = 0
        /// If _wl1 = 500, then wl1_index = 100
        const int wl1_Index = (_wl1 - sampledLambdaStart);

        /// If _wl2 = 600, then wl2_Index = 200;
        const int wl2_Index =  (_wl2 - sampledLambdaStart);

        /// Filter out the spectrum from the unwanted signals
        for (int i = 0; i < nSpectralSamples; i++)
        {
            if (i >= wl1_Index && i <= wl2_Index ) {
                c[i] = 0.0;
            }
        }
    }
    void BlockWavelength(int spectralIndex){
        c[spectralIndex] = 0;
    }
    void SetSpectrumAtSample(int i, const float value) {
        c[i] = value;
    }
    float GetSpectrumAtSample(int spectralIndex) const {
        float value =  c[spectralIndex];
        return value;
    }
    RGBSpectrum ToRGBSpectrum() const;
    static SampledSpectrum FromRGB(const float rgb[3],
        SpectrumType type = SPECTRUM_REFLECTANCE);
    static SampledSpectrum FromXYZ(const float xyz[3],
            SpectrumType type = SPECTRUM_REFLECTANCE) {
        float rgb[3];
        XYZToRGB(xyz, rgb);
        return FromRGB(rgb, type);
    }
    SampledSpectrum(const RGBSpectrum &r, SpectrumType type = SPECTRUM_REFLECTANCE);
private:
    // SampledSpectrum Private Data
    static SampledSpectrum X, Y, Z;
    static SampledSpectrum rgbRefl2SpectWhite, rgbRefl2SpectCyan;
    static SampledSpectrum rgbRefl2SpectMagenta, rgbRefl2SpectYellow;
    static SampledSpectrum rgbRefl2SpectRed, rgbRefl2SpectGreen;
    static SampledSpectrum rgbRefl2SpectBlue;
    static SampledSpectrum rgbIllum2SpectWhite, rgbIllum2SpectCyan;
    static SampledSpectrum rgbIllum2SpectMagenta, rgbIllum2SpectYellow;
    static SampledSpectrum rgbIllum2SpectRed, rgbIllum2SpectGreen;
    static SampledSpectrum rgbIllum2SpectBlue;
};


class RGBSpectrum : public CoefficientSpectrum<3> {
    using CoefficientSpectrum<3>::c;
public:
    // RGBSpectrum Public Methods
    RGBSpectrum(float v = 0.f) : CoefficientSpectrum<3>(v) { }
    RGBSpectrum(const CoefficientSpectrum<3> &v)
        : CoefficientSpectrum<3>(v) { }
    RGBSpectrum(const RGBSpectrum &s, SpectrumType type = SPECTRUM_REFLECTANCE) {
        *this = s;
    }
    static RGBSpectrum FromRGB(const float rgb[3],
            SpectrumType type = SPECTRUM_REFLECTANCE) {
        RGBSpectrum s;
        s.c[0] = rgb[0];
        s.c[1] = rgb[1];
        s.c[2] = rgb[2];
        Assert(!s.HasNaNs());
        return s;
    }
    void ToRGB(float *rgb) const {
        rgb[0] = c[0];
        rgb[1] = c[1];
        rgb[2] = c[2];
    }
    const RGBSpectrum &ToRGBSpectrum() const {
        return *this;
    }
    void ToXYZ(float xyz[3]) const {
        RGBToXYZ(c, xyz);
    }
    static RGBSpectrum FromXYZ(const float xyz[3],
            SpectrumType type = SPECTRUM_REFLECTANCE) {
        RGBSpectrum r;
        XYZToRGB(xyz, r.c);
        return r;
    }
    float y() const {
        const float YWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
        return YWeight[0] * c[0] + YWeight[1] * c[1] + YWeight[2] * c[2];
    }
    static RGBSpectrum FromSampled(const float *lambda, const float *v,
                                   int n) {
        // Sort samples if unordered, use sorted for returned spectrum
        if (!SpectrumSamplesSorted(lambda, v, n)) {
            vector<float> slambda(&lambda[0], &lambda[n]);
            vector<float> sv(&v[0], &v[n]);
            SortSpectrumSamples(&slambda[0], &sv[0], n);
            return FromSampled(&slambda[0], &sv[0], n);
        }
        float xyz[3] = { 0, 0, 0 };
        float yint = 0.f;
        for (int i = 0; i < nCIESamples; ++i) {
            yint += CIE_Y[i];
            float val = InterpolateSpectrumSamples(lambda, v, n,
                                                   CIE_lambda[i]);
            xyz[0] += val * CIE_X[i];
            xyz[1] += val * CIE_Y[i];
            xyz[2] += val * CIE_Z[i];
        }
        xyz[0] /= yint;
        xyz[1] /= yint;
        xyz[2] /= yint;
        return FromXYZ(xyz);
    }
    float Power(const int &) const { return 0.f; }
    void PrintSpectrum_f() const {
        Warning("RGB Spectrum cannot handle PrintSpectrum_f()!");
    }
    void PrintSpectrum_e() const {
        Warning("RGB Spectrum cannot handle PrintSpectrum_e()!");
    }
    void PrintSpectrum() const {
        Warning("RGB Spectrum cannot handle PrintSpectrum()!");
    }
    void PrintSpectrumTransposed_f() const {
        Warning("RGB Spectrum cannot handle PrintSpectrumTransposed_f()!");
    }
    void PrintSpectrumTransposed_e() const {
        Warning("RGB Spectrum cannot handle PrintSpectrumTransposed_e()!");
    }
    void PrintSpectrumTransposed() const {
        Warning("RGB Spectrum cannot handle PrintSpectrumTransposed()!");
    }
    void PrintSpectrumDistribution_f() const {
        Warning("RGB Spectrum cannot handle PrintSpectrumTransposed()!");
    }
    void PrintSpectrumDistribution_e() const {
        Warning("RGB Spectrum cannot handle PrintSpectrumDistribution_e()!");
    }
    void PrintSpectrumDistribution() const {
        Warning("RGB Spectrum cannot handle PrintSpectrumDistribution()!");
    }
    void PrintSpectrumDistributionTransposed_f() const {
        Warning("RGB Spectrum cannot handle PrintSpectrumDistributionTransposed_f()!");
    }
    void PrintSpectrumDistributionTransposed_e() const {
        Warning("RGB Spectrum cannot handle PrintSpectrumDistributionTransposed_e()!");
    }
    void PrintSpectrumDistributionTransposed() const {
        Warning("RGB Spectrum cannot handle PrintSpectrumDistributionTransposed()!");
    }
    void PrintNonZeroSpectrum_f() const {
        Warning("RGB Spectrum cannot handle PrintNonZeroSpectrum_f()!");
    }
    void PrintNonZeroSpectrum_e() const {
        Warning("RGB Spectrum cannot handle PrintNonZeroSpectrum_e()!");
    }
    void PrintNonZeroSpectrum() const {
        Warning("RGB Spectrum cannot handle PrintNonZeroSpectrum()!");
    }
    void PrintNonZeroSpectrumTransposed_f() const {
        Warning("RGB Spectrum cannot handle PrintNonZeroSpectrumTransposed_f()!");
    }
    void PrintNonZeroSpectrumTransposed_e() const {
        Warning("RGB Spectrum cannot handle PrintNonZeroSpectrumTransposed_e()!");
    }
    void PrintNonZeroSpectrumTransposed() const {
        Warning("RGB Spectrum cannot handle PrintNonZeroSpectrumTransposed()!");
    }
    void SetToBlack() { c[0] = 0.f; c[1] = 0.f; c[2] = 0.f; }
    void SetToOne() { c[0] = 1.f; c[1] = 1.f; c[2] = 1.f; }
    void SetToUniformValue(const float value) {
        c[0] = value; c[1] = value; c[2] = value;
    }
    void SetSampleValueAtWavelengthIndex(int lambdaIdx, float value) {}
    void SetSampleValueAtWavelength(int lambda, float value) {}
    float GetSampleValueAtWavelength(int lambda) const { return 0.f; }
    float GetSampleValueAtWavelengthIndex(int lambdaIdx) const { return 0.f; }
    float& GetRefToSampleValueAtWavelengthIndex(int lambdaIdx) {}
    float CalculateSPDPower(void) const {}
    void NormalizeSPDArea() {}
    void Normalize() {}
    int GetLaserWavelengthIndex() const { return -1; }
    int GetLaserWavelength() const { return -1; }
    bool VerifyLaser(int &wavelength, int & wavelengthIndex) const {
        return false;
    }
    float GetLaserEmissionPower() const { return 0.f; }
    float GetLaserEmissionPower(const int lambdaIndex) const {
        return 0.f;
    }
    void BlockBand(const int wl1, const int wl2) {}
    void BlockWavelength(int spectralIndex){}
    void SetSpectrumAtSample(int i, const float value) {}
    float GetSpectrumAtSample(int spectralIndex) const { return 0.f; }
    static RGBSpectrum LoadSampledSpectrum(const float *lambda,
                const float *v, int n) {
        Warning("RGB Spectrum cannot handle LoadSampledSpectrum()!");
        return RGBSpectrum(0.0);
    }
};



// Spectrum Inline Functions
template <int nSamples> inline CoefficientSpectrum<nSamples>
Pow(const CoefficientSpectrum<nSamples> &s, float e) {
    CoefficientSpectrum<nSamples> ret;
    for (int i = 0; i < nSamples; ++i)
        ret.c[i] = powf(s.c[i], e);
    Assert(!ret.HasNaNs());
    return ret;
}


inline Spectrum Lerp(float t, const Spectrum &s1, const Spectrum &s2) {
    return (1.f - t) * s1 + t * s2;
}

}

#endif // PBRT_CORE_SPECTRUM_H
