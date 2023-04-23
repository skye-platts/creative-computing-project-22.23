//
//  fftProcessor.cpp
//  Phase Vocoder 17_04_23
//
//  Created by Skye Platts on 17/04/2023.
//

#include "fftProcessor.hpp"


fftProcessor::fftProcessor()
{
}

fftProcessor::~fftProcessor()
{
    fftw_cleanup();
}

//--------------------------------------------------
void fftProcessor::setup(unsigned int win, unsigned int hop, unsigned int circBuffSize)
{
    fftSize = win;
    hopSize = hop;
    circularBufferSize = circBuffSize;
    
    // Hanning window
    windowFunction.resize(fftSize);
    for (unsigned int i = 0; i < fftSize; i++) {
        windowFunction[i] = 0.5f * (1.0f - cosf(2.0f * M_PI * (float)i / (float)(fftSize - 1)));
    }
    
    binFrequencies.resize(fftSize / 2 + 1);
    for(int n = 0; n <= fftSize/2; n++) {
        binFrequencies[n] = 2.0 * M_PI * (float)n / (float)fftSize;
    }
    
    fftwUnwrappedBuffer = fftw_alloc_real(fftSize*sizeof(double));
    fftwAnalysisComplex = fftw_alloc_complex(fftSize*sizeof(double));
    fftwSynthesisComplex = fftw_alloc_complex(fftSize*sizeof(double));
    fftwSynthesisReal = fftw_alloc_real(fftSize*sizeof(double));
    
}

//--------------------------------------------------
void fftProcessor::setPitchShift(float factor)
{
    pitchShift = factor;
}

//--------------------------------------------------
float fftProcessor::wrapPhase(float phaseIn)
{
    if (phaseIn >= 0) {
        return fmodf(phaseIn + M_PI, 2.f * M_PI) - M_PI;
    } else {
        return fmodf(phaseIn - M_PI, -2.f * M_PI) + M_PI;
    }
}


//--------------------------------------------------
void fftProcessor::process(std::vector<float> const& inBuffer, unsigned int inWritePointer, std::vector<float> &outBuffer, unsigned int &outWritePointer)
{
    
    static std::vector<float> unwrappedBuffer(fftSize);
    
    static std::vector<float> lastInputPhases(fftSize);
    static std::vector<float> lastOutputPhases(fftSize);
    
    static std::vector<float> analysisMagnitudes(fftSize / 2 + 1);
    static std::vector<float> analysisFrequencies(fftSize / 2 + 1);
    static std::vector<float> synthesisMagnitudes(fftSize / 2 + 1);
    static std::vector<float> synthesisFrequencies(fftSize / 2 + 1);
    
    
    for (int n = 0; n < fftSize; n++) {
        // Calculate index for write in circular input buffer
        int circularBufferIndex = (inWritePointer + n - fftSize + circularBufferSize) % circularBufferSize;
        unwrappedBuffer[n] = inBuffer[circularBufferIndex] * windowFunction[n];
    }
    for (int n = 0; n < fftSize; n++) {
        fftwUnwrappedBuffer[n] = unwrappedBuffer[n];
    }
    
    
    fftw_plan fftPlan = fftw_plan_dft_r2c_1d(fftSize, fftwUnwrappedBuffer, fftwAnalysisComplex, FFTW_ESTIMATE);
    fftw_execute(fftPlan);
    fftw_destroy_plan(fftPlan);
    
    
    for (int n = 0; n <= fftSize / 2; n++) {
        
        float magnitude = sqrtf(pow(fftwAnalysisComplex[n][REAL], 2) + pow(2, fftwAnalysisComplex[n][IMAG]));
        float phase = atan2(fftwAnalysisComplex[n][IMAG], fftwAnalysisComplex[n][REAL]);
        
        // Calculate phase difference for bin[n] between last and current hop
        float phaseDifference = wrapPhase(phase - lastInputPhases[n]);
    
        // True bin phase difference = phase difference between hop - expected phase difference...
        // ... for centre frequency
        float binCentreFrequency = 2.0f * M_PI * (float)n / (float)fftSize;
        phaseDifference = wrapPhase(phaseDifference - binCentreFrequency * hopSize);
        
        // Calculate drift from centre frequency as a fraction of the number of bins
        float binDrift = phaseDifference * (float)fftSize / (float)hopSize / (2.0f * M_PI);
        
        // Add original bin index to get the bin where energy is as a fraction of the number of bins
        analysisFrequencies[n] = (float)n + binDrift;
        
        analysisMagnitudes[n] = magnitude;
        
        // Remember phases from this hop
        lastInputPhases[n] = phase;
        
    }
    
    // Clear synthesis parameters, save computation cost using half fft for now
    // Conjugate will be overwritten later
    for (int n = 0; n <= fftSize / 2; n++) {
        synthesisMagnitudes[n] = synthesisFrequencies[n] = 0;
    }
    
    for (int n = 0; n <= fftSize / 2; n++) {
        // Calculate bin index to store shifted energy
        int newBin = floorf(n * pitchShift + 0.5);
        
        if (newBin <= fftSize / 2) {
            synthesisMagnitudes[newBin] += analysisMagnitudes[n];
            // Shift frequency
            synthesisFrequencies[newBin] = analysisFrequencies[n] * pitchShift;
        }
    }
    
    for (int n = 0; n <= fftSize / 2; n++) {
        
        float magnitude = synthesisMagnitudes[n];
        
        // Offset from centre frequency of bin as a fraction of the number of bins
        float binDrift = synthesisFrequencies[n] - n;
        
        //  Convert to phase value
        float phaseDifference = binDrift * 2.0f * M_PI * (float)hopSize / (float)fftSize;
        
        // Add back the expected phase difference for the bin calculated from its centre frequency
        float binCentreFrequency = 2.0f * M_PI * (float)n / (float)fftSize;
        phaseDifference += binCentreFrequency * hopSize;
        
        // Increment phase from last hop
        float outPhase = wrapPhase(lastOutputPhases[n] + phaseDifference);
        
        // Trigonometry for polar to complex
        fftwSynthesisComplex[n][REAL] = magnitude * cosf(outPhase);
        fftwSynthesisComplex[n][IMAG] = magnitude * sinf(outPhase);
        
        // Overwrite complex conjugate
        if (n > 0 && n < fftSize / 2) {
            fftwSynthesisComplex[fftSize-n][REAL] = fftwSynthesisComplex[n][REAL];
            fftwSynthesisComplex[fftSize-n][IMAG] = -fftwSynthesisComplex[n][IMAG];
        }
        
        lastOutputPhases[n] = outPhase;
        
    }
    
    fftw_plan ifftPlan = fftw_plan_dft_c2r_1d(fftSize, fftwSynthesisComplex, fftwSynthesisReal, FFTW_ESTIMATE);
    fftw_execute(ifftPlan);
    fftw_destroy_plan(ifftPlan);
    
    for (int n = 0; n < fftSize; n++) {
        fftwSynthesisReal[n] /= fftSize;
    }
    
    for (int n = 0; n < fftSize; n++) {
        int circularBufferIndex = (outWritePointer + n - fftSize + circularBufferSize) % circularBufferSize;
        outBuffer[circularBufferIndex] += fftwSynthesisReal[n] * windowFunction[n];
    }

    outWritePointer = (outWritePointer + hopSize) % circularBufferSize;
}
