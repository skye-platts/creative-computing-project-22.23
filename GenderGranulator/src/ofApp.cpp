#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    // Setup OF SoundStream and ofxMaxim settings
    soundStream.setup(2, 1, sampleRate, bufferSize, 4);
    ofxMaxiSettings::setup(sampleRate, 2, bufferSize);
    
    // Setup a maxiSample to store incoming audio
    inSample.setLength(131072);
    audioInWritePointer = 0;
    outMaxReadPointer = (inSample.getLength() / bufferSize) - 1;
    
    // Initialise grains
    numGrains = 5;
    grainSpread = 1;
    grains.resize(numGrains);
    grainOutputs.resize(numGrains);
    grainPositionPointers.resize(numGrains);
    for (int i = 0; i < numGrains; i++) {
        grains[i] = new maxiTimePitchStretch<grainPlayerWindowFunction, maxiSample>(&inSample);
        double panOutput[2];
        grainPanOutputs.push_back(panOutput);
        grainPositionPointers[i] = outMaxReadPointer;
    }
    
    // Initialise grain position just behind write index
    for (int i = 0; i < numGrains; i++) {
        grainPositionPointers[i] = outMaxReadPointer - (i*grainSpread);
    }
    
    // FFT setup
    fftSize = 1024;
    fft.setup(fftSize, 512, 256);
    
}

//--------------------------------------------------------------
// grain->play takes a length in seconds to 3 decimal places
float roundMillis(float millis, int bufferSize, int sampleRate) {
    return std::ceil(((millis*bufferSize)/sampleRate) * 1000.f) / 1000.f;
}

//--------------------------------------------------------------
void ofApp::update(){
    
}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::exit(){
    
    for (int i = 0; i < numGrains; i++) {
        delete grains[i];
    }
    
    // Free sample memory
    inSample.clear();
    
    soundStream.close();
    
}


//--------------------------------------------------------------
// Wrap phase -pi -> pi
float wrapPhase(float phaseIn)
{
    if (phaseIn >= 0) {
        return fmodf(phaseIn + M_PI, 2.f * M_PI) - M_PI;
    } else {
        return fmodf(phaseIn - M_PI, -2.f * M_PI) + M_PI;
    }
}

//--------------------------------------------------------------
void ofApp::audioIn(float * input, int bufferSize, int nChannels){
    
    // Last frame
    static std::vector<float> lastInputPhases(fftSize / 2 + 1);
    static std::vector<float> analysisFrequencies(fftSize / 2 + 1);
    
    // Circular buffer of estimate frequencies for average smoothing
    static std::vector<float> fundamentalFreqAvgBuffer(40);
    static int frameCount = 0;
    
    float currentFrameFreq = 0;
    

    for (int i = 0; i < bufferSize; i++) {
        
        // Record live input into buffer/maxiSample
        inSample.loopRecord(input[i], true, 0.f);
        
        // When fft output is ready, process
        if (fft.process(input[i])) {

            int hottestBinIndex = 0;
            float maxBinMagnitude = 0;
             
            // Running phase for each bin to calculate estimate of exact frequency
            
            for (int n = 0; n <= fftSize/2; n++) {
                
                float phaseDifference = fft.phases[n] - lastInputPhases[n];
                
                // True bin phase difference = phase difference between hop - expected phase difference...
                // ... for centre frequency
                float binCentreFrequency = 2.0 * M_PI * (float)n / (float)fftSize;

                phaseDifference = wrapPhase(phaseDifference - binCentreFrequency * fft.hopSize);
                
                // Calculate drift from centre frequency as a fraction of the number of bins
                float binDeviation = phaseDifference * (float)fftSize / (float)fft.hopSize / (2.0 * M_PI);
                
                // Add original bin index to get the bin where energy is as a fraction of the number of bins
                analysisFrequencies[n] = (float)n + binDeviation;

                // Remember for next window
                lastInputPhases[n] = fft.phases[n];
                
                // Get index of hottest bin
                if (fft.magnitudes[n] > maxBinMagnitude) {
                    maxBinMagnitude = fft.magnitudes[n];
                    hottestBinIndex = n;
                }

            }
            
            // Calculate frequency from bin fraction
            currentFrameFreq = (analysisFrequencies[hottestBinIndex] * (float)sampleRate) / (float)fftSize;
            
            // Filter frequencies above fundamental range from circular buffer of averages to avoid high frequency estimates whenever speech is sibiliant
            if (currentFrameFreq > 1000.f) {
                int lastFrameIndex = frameCount - 1;
                if (lastFrameIndex < 0) {
                    lastFrameIndex = fundamentalFreqAvgBuffer.size() - abs(lastFrameIndex);
                }
                currentFrameFreq = fundamentalFreqAvgBuffer[lastFrameIndex];
            }
            

        }
    }
    
    // Push current frequency estimate into circular buffer for average
    fundamentalFreqAvgBuffer[frameCount] = currentFrameFreq;
    
    // Calculate average across 40 audio stream buffers
    fundamentalFreq = std::accumulate(fundamentalFreqAvgBuffer.begin(), fundamentalFreqAvgBuffer.end(), 0.0f) / (float)fundamentalFreqAvgBuffer.size();
    
    // Clamp fundamental frequency values for human speech fundamental range
    fundamentalFreq = ofClamp(fundamentalFreq, 100.f, 400.f);
    
    // Modulate granulator parameters by fundamental frequency
    double grainModulator = ofMap(fundamentalFreq, 100.f, 400.f, 4, 0.01, true);
    grainLength = roundMillis(grainModulator, bufferSize, sampleRate);
    stereoSpread = ofMap(fundamentalFreq, 100.f, 400.f, 0.f, 1.f, true);
    volumeMod = ofMap(fundamentalFreq, 100.f, 400.f, 1.f, 0.65, true);
    grainSpread = ofMap(fundamentalFreq, 100.f, 400.f, 1.f, 10.f, true);
    
    
    // Update grainPosition behind the live input write pointer
    for (int i = 0; i < numGrains; i++) {
        grainPositionPointers[i] = outMaxReadPointer - (i*grainSpread);
    }

    
    // Manage wrapping of pointers for circular buffers etc.
    if (++frameCount >= fundamentalFreqAvgBuffer.size()) {
        frameCount = 0;
    }
    if (++audioInWritePointer * bufferSize >= inSample.getLength()) {
        audioInWritePointer = 0;
    }
    if (++outMaxReadPointer * bufferSize >= inSample.getLength()) {
        outMaxReadPointer = 0;
    }
    for (int i = 0; i < numGrains; i++) {
        if ((++grainPositionPointers[i] * bufferSize) >= inSample.getLength()) {
            double remain = grainPositionPointers[i] - (inSample.getLength() / bufferSize);
            grainPositionPointers[i] = 0 + remain;
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::audioOut(float * output, int bufferSize, int nChannels){
    
    for (int i = 0; i < bufferSize; i++) {
        
        double stereoFactor = stereoSpread/((float)numGrains - 1.0f);
        
        // Fetch grain output buffers
        for (int i = 0; i < numGrains; i++) {
            double currentStereoOutput[2];
            double sampleCurrentPos = ((double(grainPositionPointers[i]) * double(bufferSize)) / double(inSample.getLength()));
            grains[i]->setPosition(sampleCurrentPos);
            grainOutputs[i] = grains[i]->play(1.0, 1.0, grainLength, 4);
            // Scale signal for num grains
            grainOutputs[i] *= 1.f/numGrains;
            
            // Stereo spread
            double stereoPos;
            if (i != 0) {
                if (i % 2 == 0) {
                    if (i == 2) {
                        stereoPos = 0.5 + stereoFactor;
                    } else {
                        stereoPos = 0.5 + (stereoFactor * 2);
                    }
                } else {
                    if (i == 1) {
                        stereoPos = 0.5 - stereoFactor;
                    } else {
                        stereoPos = 0.5 - (stereoFactor * 2);
                    }
                }
            } else {
                stereoPos = 0.5;
            }
            
            mixer.stereo(grainOutputs[i], currentStereoOutput, stereoPos);
            grainPanOutputs[i] = currentStereoOutput;
        }
        
        stereoOutputs[0] = stereoOutputs[1] = 0;
        
        for (int i = 0; i < numGrains; i++) {
            double* panOutput = grainPanOutputs[i];
            stereoOutputs[0] += panOutput[0];
            stereoOutputs[1] += panOutput[1];
        }
        
        
        output[i*nChannels] = stereoOutputs[0] * volumeMod;
        output[i*nChannels+1] = stereoOutputs[1] * volumeMod;
        
    }
    
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
