# creative-computing-project-22.23

## maxMSP-FLUCOMA-mlp_experiments


mlpSpeechRec.maxpat (MAX-MSP) - Experiments using audio feature extraction to train FLUCOMA multilayer perceptron for speaker gender recognition. v2 uses audio downsampled to a rate of 16000 and shorter windows of 512 samples with less averaging to yield an accuracy of 100% (training) and 97% (validation) compared to 77% and 57% with attained using v1.
Save training dataset in max search path.

Training and test audio from The Common Voice dataset https://commonvoice.mozilla.org/en/
For reference to original Commmon Voice identifiers for each audio file, consult speechsamplesRef.rtf

Requirements:
FLUCOMA - https://www.flucoma.org/download/

