#pragma once

#include "tsingou.hpp"

class Synth { 
public:

    enum class ParamId: unsigned int { Beta, Epsilon, Rho, Count };

 private:
    // string models
    Tsingou t[2];

    // stereo spread per string
    float spread = 0.75f;
    // post-spread anti-width
    float mono = 0.f;

    // each string has 2 pickup positions
    static constexpr int pos1 = Tsingou::NUM_MASSES / 4;
    static constexpr int pos2 = Tsingou::NUM_MASSES / 4 * 3;

    static void performStereoSpread(float in11, float in12, float in21, float in22, 
        float spread, float* out1, float* out2) { 
        *out1 = in11;
        *out2 = in22;
        *out1 += in12 * (1.f - spread);
        *out2 = in12 * spread;
        *out2 += in21 * (1.f - spread);
        *out1 = in21 * spread;
    }

public:
    void processAudioBlockInterleaved(const float* input, float* output, int numFrames) {
        float* dst = output;
        const float *src = input;
        float l, r;
        for (int frame = 0; frame < numFrames; ++frame) {
            t[0].process(*src++);
            t[1].process(*src++);

            const float out11 = t[0].get_output(pos1);
            const float out12 = t[0].get_output(pos2);
            const float out21 = t[1].get_output(pos1);
            const float out22 = t[1].get_output(pos2);

            performStereoSpread(out11, out12, out21, out22, spread, &l, &r);
            *dst++ = l + (mono * (r-l));
            *dst++ = r + (mono * (l-r));
        }
    }

    void pluck(unsigned int string, unsigned int pos=(Tsingou::NUM_MASSES/4), float amp=1.f) {
        if (string > 1) { return; }
        if (pos >= TSINGOU_NUM_MASSES) { return; }
        t[string].set_pos(pos, amp);
    }

    void setParam(unsigned int string, unsigned int id) { 
        if (string > 1) { return; }
        if (id >= (int)ParamId::Count) { return; }
        switch ((ParamId)id) {
            case ParamId::Beta: 
                t[string].set_beta();
                break;
            case ParamId::Epsilon:
                t[string].set_epsilon();
                break;
            case ParamId::Rho:
                t[string].set_rho();
                break;
        }
    }
}