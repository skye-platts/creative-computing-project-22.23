#pragma once

#include "ofMain.h"
#include "ofxMaxim.h"
#include "maxiGrains.h"
#include <sys/time.h>
#include <numeric> //accumulate

// hanning window for grains
typedef hannWinFunctor grainPlayerWindowFunction;

class ofApp : public ofBaseApp{

	public:
    
		void setup();
		void update();
		void draw();
        void exit();
    
        void audioIn(float * input, int bufferSize, int nChannels);
        void audioOut(float * buffer, int bufferSize, int nChannels);

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
private:

    // Audio settings
    ofSoundStream soundStream;
    int sampleRate = 44100;
    int bufferSize = 512;
    double outputSignal;
    double stereoOutputs[2];
    
    // Input buffer
    maxiSample inSample;
    unsigned int audioInWritePointer;
    float outMaxReadPointer;
    
    // Granular synthesizer
    unsigned int numGrains;
    std::vector<maxiTimePitchStretch<grainPlayerWindowFunction, maxiSample>*> grains;
    std::vector<double> grainOutputs;
    maxiMix mixer;
    std::vector<double*> grainPanOutputs;
    
    // Granular synthesizer parameters
    std::vector<double> grainPositionPointers;
    double grainSpread;
    double grainLength;
    double stereoSpread;
    double volumeMod;

    // Frequency tracking
    maxiFFT fft;
    unsigned fftSize;
    float fundamentalFreq;
    
		
};
