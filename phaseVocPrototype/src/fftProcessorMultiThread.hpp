//
//  fftProcessorMultiThread.hpp
//  Phase Vocoder 17_04_23
//
//  Created by Skye Platts on 17/04/2023.
//
 
#ifndef fftProcessorMultiThread_hpp
#define fftProcessorMultiThread_hpp

#include <stdio.h>

#include "ofMain.h"
#include <fftw3.h>
#include <algorithm>

#endif /* fftProcessorMultiThread_hpp */

class fftProcessorMultiThread : public ofThread {

#define REAL 0
#define IMAG 1
    
public:
    
    fftProcessorMultiThread();
    ~fftProcessorMultiThread();
    
    void setup(unsigned int win, unsigned int hop, unsigned int circBuffSize);
    
    void setPitchShift(float factor);
    
    float wrapPhase(float phaseIn);

    void prepFFT(std::vector<float> &inBuffer, unsigned int inputPointer, std::vector<float> &outBuffer, unsigned int outWritePointer);
                 
    void threadedFunction();
    
    void process(std::vector<float> const& inBuffer, unsigned int inWritePointer, std::vector<float> &outBuffer, unsigned int &outWritePointer);
    
    
    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;
    unsigned int outputWritePointer;
    
    
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
    
    
    unsigned int inputReadPointer;

};
