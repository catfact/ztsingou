#include "tsingou.h"

class Synth { 
    Tsingou t[2];

    void pluck(int i) { 
        t[i].pluck();
    }
    void processAudioBlock(const float* input, float* output);
}