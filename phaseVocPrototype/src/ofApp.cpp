#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    soundStream.setup(2, 1, sampleRate, bufferSize, 4);
    
    fftSize = 1024;
    hopSize = 128;
    circularBufferSize = (fftSize + hopSize) * 4;
    
    fft.setup(fftSize, hopSize, circularBufferSize);
    fft.setPitchShift(2.f);
    
//    // Multi thread attempt
//    fftMultiThread.setup(fftSize, hopSize, circularBufferSize); //multi thread
//    fftMultiThread.setPitchShift(2.f);

    scaleFactor = 0.5;
    
    audioInputBuffer.resize(soundStream.getBufferSize()*soundStream.getNumInputChannels());
    circularInputBuffer.resize(circularBufferSize);
    circularOutputBuffer.resize(circularBufferSize);
    inputBufferPointer = 0;
    outputBufferWritePointer = fftSize + (2 * hopSize);
    outputBufferReadPointer = 0;
    hopCounter = 0;
    
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::exit(){
    soundStream.close();
}

//--------------------------------------------------------------
void ofApp::audioIn(float * input, int bufferSize, int nChannels){
    
    for (int i = 0; i < bufferSize; i++) {
        audioInputBuffer[i] = input[i];
    }
    
}

//--------------------------------------------------------------
void ofApp::audioOut(float * buffer, int bufferSize, int nChannels){
    
//    // Multi thread attempt
//    ofScopedLock fftLock(fftMutex);
    
    for (int n = 0; n < bufferSize; n++) {
        // Read sample from input buffer
        float in = audioInputBuffer[n];
        
        circularInputBuffer[inputBufferPointer++] = in;
        if (inputBufferPointer >= circularBufferSize) {
            // Wrap circular input buffer
            inputBufferPointer = 0;
        }
        
        // Fetch output from frequency processing circular output buffer at read pointer
        float out = circularOutputBuffer[outputBufferReadPointer];
        
        // Clear value for next overlap-add
        circularOutputBuffer[outputBufferReadPointer] = 0;
        
        out *= (float)hopSize / (float)fftSize;
//        out *= scaleFactor;
        
        outputBufferReadPointer++;
        if (outputBufferReadPointer >= circularBufferSize) {
            outputBufferReadPointer = 0;
        }
        
        hopCounter++;
        if (hopCounter >= hopSize) {
            hopCounter = 0;
            fft.process(circularInputBuffer, inputBufferPointer, circularOutputBuffer, outputBufferWritePointer);
            
            
//            // Terrible attempt at multi-threading i have nooo idea what i'm doing :/
//            if (!fftMultiThread.isThreadRunning()) {
//                fftMutex.unlock();
//                fftMultiThread.prepFFT(circularInputBuffer, inputBufferPointer, circularOutputBuffer, outputBufferWritePointer);
//                fftMutex.lock();
//                fftMultiThread.startThread();
//            }
//            if (!fftMultiThread.isThreadRunning()) {
//                circularOutputBuffer.assign(fftMultiThread.outputBuffer.begin(), fftMultiThread.outputBuffer.end());
//                outputBufferWritePointer = fftMultiThread.outputWritePointer;
//            }

        }

        buffer[n*nChannels] = out;
        buffer[n*nChannels+1] = out;
        
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
