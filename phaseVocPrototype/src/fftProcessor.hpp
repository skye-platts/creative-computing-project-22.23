//
//  fftProcessor.hpp
//  Phase Vocoder 17_04_23
//
//  Created by Skye Platts on 17/04/2023.
//

#ifndef fftProcessor_hpp
#define fftProcessor_hpp

#include <stdio.h>

#include "ofMain.h"
#include <fftw3.h>

#endif /* fftProcessor_hpp */


class fftProcessor {

#define REAL 0
#define IMAG 1
    
public:
    
    fftProcessor();
    ~fftProcessor();
    
    void setup(unsigned int win, unsigned int hop, unsigned int circBuffSize);
    
    void setPitchShift(float factor);
    
    float wrapPhase(float phaseIn);
    
    void process(std::vector<float> const& inBuffer, unsigned int inWritePointer, std::vector<float> &outBuffer, unsigned int &outWritePointer);
    
    void threadedFunction();
    
    
private:
    
    unsigned int fftSize;
    unsigned int hopSize;
    unsigned int circularBufferSize;
    float pitchShift;
    
    std::vector<float> windowFunction;
    std::vector<float> binFrequencies;
    
    double *fftwUnwrappedBuffer;
    fftw_complex *fftwAnalysisComplex;
    fftw_complex *fftwSynthesisComplex;
    double *fftwSynthesisReal;
    
    
};
