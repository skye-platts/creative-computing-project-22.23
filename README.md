# creative-computing-project-22.23
## phase-vocoder-experiments

Experiments in phase vocoder effect implementation


nonRealTimePitchShift.ipynb (PYTHON) - non real-time implementation of overlap-add phase vocoder pitch shift. Change argument for "wavReadMono" to read new file. 
___________________________


phaseVocPrototype (C++) - unsuccessful prototype implementation of real-time phaseVocoder in OpenFrameworks. The frequency domain maths needs to be executed on another thread with a lower priority than the main audio thread. My attempt can be found in fftProcessorMultiThread.h/.cpp however, this is beyond the scope of my current skillset.

Requirements:
fftw library - https://www.fftw.org
___________________________
