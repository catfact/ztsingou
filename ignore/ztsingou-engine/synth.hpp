#pragma once

#include "tsingou.hpp"

class Synth {
public:

    enum class ParamId : unsigned int {
        Amp = 0,
        Spread = 1,
        Mono = 2,
        Gain = 3,
        PickupPos1 = 4,
        PickupPos2 = 5,
        ExcitePos = 6,
        Beta = 7,
        Epsilon = 8,
        Rho = 9,
        Pluck = 10,
        Count
    };

private:

    // default parameters
    static constexpr double sr_default = 48000.0;
    // time step of the simulation
    static constexpr double dt = 0.000001;
    // iterations per sample
    static constexpr int ips = 32;

    // string models
    Tsingou t[2];

    // amplitude per string
    float amp[2] = {0, 0};
    // stereo spread per string
    float spread = 0.75f;
    // post-spread anti-width
    float mono = 0.f;
    // input amplitude
    float gain = 0.f;

    // pickup positions, 2 per string
    float pickupPos[2][2];
    // excitation positions, 1 per string
    float excPos[2];

    /// TODO: would be nice to have built-in smoothers/slew limiters for many/all params

    static void performStereoSpread(float in11, float in12, float in21, float in22,
                                    float spread, float *out1, float *out2) {
        *out1 = in11;
        *out2 = in22;
        *out1 += in12 * (1.f - spread);
        *out2 = in12 * spread;
        *out2 += in21 * (1.f - spread);
        *out1 = in21 * spread;
    }

public:
    Synth() {
        for (int i = 0; i < 2; ++i) {
            t[i].init(dt, sr_default);
            t[i].set_ips(ips);
            t[i].clear_state();
        }

        pickupPos[0][0] = (float)Tsingou::NUM_MASSES / 4;
        pickupPos[0][1] = pickupPos[0][0] * 3;
        pickupPos[1][0] = pickupPos[0][0];
        pickupPos[1][1] = pickupPos[0][1];

        excPos[0] = Tsingou::NUM_MASSES / 2;
        excPos[1] = excPos[0];
    }

//    void processAudioBlockInterleaved(const float *input, float *output, int numFrames) {
//        float *dst = output;
//        const float *src = input;
//        float l, r;
//        for (int frame = 0; frame < numFrames; ++frame) {
//            t[0].update(*src++);
//            t[1].update(*src++);
//
//            const auto out11 = (float) t[0].get_output(pickupPos[0][0]);
//            const auto out12 = (float) t[0].get_output(pickupPos[0][1]);
//            const auto out21 = (float) t[1].get_output(pickupPos[1][0]);
//            const auto out22 = (float) t[1].get_output(pickupPos[1][1]);
//
//            performStereoSpread(out11, out12, out21, out22, spread, &l, &r);
//            *dst++ = l + (mono * (r - l));
//            *dst++ = r + (mono * (l - r));
//        }
//    }

    void processAudioBlockNonInterleaved(const float *input[2], float *output[2], int numFrames) {
        float l, r;
        for (int frame = 0; frame < numFrames; ++frame) {
            t[0].update(input[0][frame] * gain);
            t[1].update(input[1][frame] * gain);

#if 1
            l = (float)t[0].get_output(pickupPos[0][0]) * amp[0];
            r = (float)t[1].get_output(pickupPos[1][0]) * amp[1];
            output[0][frame] = l + (mono * (r - l));
            output[1][frame] = r + (mono * (l - r));
#else
            const auto out11 = (float)t[0].get_output(pickupPos[0][0]) * amp[0];
            const auto out12 = (float)t[0].get_output(pickupPos[0][1]) * amp[0];
            const auto out21 = (float)t[1].get_output(pickupPos[1][0]) * amp[1];
            const auto out22 = (float)t[1].get_output(pickupPos[1][1]) * amp[1];

            performStereoSpread(out11, out12, out21, out22, spread, &l, &r);
            output[0][frame] = l + (mono * (r - l));
            output[1][frame] = r + (mono * (l - r));
#endif
        }
    }

    void pluck(unsigned int string, unsigned int pos = (Tsingou::NUM_MASSES / 4), float amp = 1.f) {
        if (string > 1) { return; }
        if (pos >= Tsingou::NUM_MASSES) { return; }
        if (pos < 1) { return; }
        t[string].set_pos((int)pos, amp);
    }

    void setParam(unsigned int string, int id, float value) {
        if (string > 1) { return; }
        if (id < 0) return;
        if (id >= (int) ParamId::Count) { return; }
        switch ((ParamId) id) {
            case ParamId::Amp:
                amp[string] = value;
                break;
            case ParamId::Spread:
                /// FIXME: i guess better have separate `setParam` and `setStringParam` methods
                spread = value;
                break;
            case ParamId::Mono:
                mono = value;
                break;
            case ParamId::Gain:
                gain = value;
                break;
            case ParamId::PickupPos1:
                pickupPos[string][0] = value;
                break;
            case ParamId::PickupPos2:
                pickupPos[string][1] = value;
                break;
            case ParamId::ExcitePos:
                excPos[string] = value;
                break;
            case ParamId::Beta:
                t[string].set_beta(value);
                break;
            case ParamId::Epsilon:
                t[string].set_epsilon(value);
                break;
            case ParamId::Rho:
                t[string].set_rho(value);
                break;
            case ParamId::Pluck:
                // FIXME: might be nice to interpolate fractional exc. pos.
                this->pluck(string, (unsigned int) excPos[string], value);
                break;
            case ParamId::Count:
            default:;
                break;
        }
    }
};