#pragma once

#include "ofMain.h"

#include "ofxMaxim.h"
#include <fftw3.h>
#include <vector>
#include <algorithm>

#include "fftProcessor.hpp"
#include "fftProcessorMultiThread.hpp"

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
    
    ofSoundStream soundStream;
    ofSoundStreamSettings streamSettings;
    int sampleRate = 44100;
    int bufferSize = 1024;
    std::vector<float> audioInBuffer;
    
    
    unsigned int fftSize;
    unsigned int hopSize;
    unsigned int hopCounter;
    
    std::vector<float> audioInputBuffer;
    unsigned int circularBufferSize;
    std::vector<float> circularInputBuffer;
    unsigned int inputBufferPointer;
    std::vector<float> circularOutputBuffer;
    unsigned int outputBufferReadPointer;
    unsigned int outputBufferWritePointer;
    
    fftProcessor fft;
    
    float scaleFactor;
    
//    // Multi thread attempt
//    fftProcessorMultiThread fftMultiThread;
//    ofMutex fftMutex;
    
};
