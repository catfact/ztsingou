#pragma once

#include "tsingou.hpp"

class Synth {
public:

    /// TODO: could use a couple other parameters:
    /// - iterations per sample (`ips`)
    /// - mass count (`mc`; keep `x[mc]` at zero, and don't process `x[i]` for `i>mc`)

    enum class ParamStringId : unsigned int {
        Amp = 0,
        PickupPos1 = 1,
        PickupPos2 = 2,
        ExcitePos = 3,
        Beta = 4,
        Epsilon = 5,
        Rho = 6,
        Pluck = 7,
        Count = 8
    };

    enum class ParamGlobalId : unsigned int {
        Spread = 0,
        Mono = 1,
        Gain = 2,
        Ips = 3,
        Masses = 4,
        Count = 5
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
        *out2 += in12 * spread;
        *out2 += in21 * (1.f - spread);
        *out1 += in21 * spread;
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
            t[0].add_to_pos(excPos[0], input[0][frame] * gain);
            t[1].add_to_pos(excPos[1], input[1][frame] * gain);
            t[0].update();
            t[1].update();

#if 0
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

    void setParamGlobal(int id, float value) {
        if (id < 0) return;
        if (id >= (int) ParamGlobalId::Count) { return; }
        switch ((ParamGlobalId) id) {
            case ParamGlobalId::Spread:
                spread = value;
                break;
            case ParamGlobalId::Mono:
                mono = value;
                break;
            case ParamGlobalId::Gain:
                gain = value;
                break;
            case ParamGlobalId::Ips:
                t[0].set_ips((int)value);
                t[1].set_ips((int)value);
                break;
            case ParamGlobalId::Masses:
                if (value > 0 && value < Tsingou::NUM_MASSES) {
                    t[0].set_masses((int)value);
                    t[1].set_masses((int)value);
                }
                break;
            case ParamGlobalId::Count:
                break;
        }
    }

        void setParamString(unsigned int string, int id, float value) {
        if (string > 1) { return; }
        if (id < 0) return;
        if (id >= (int) ParamStringId::Count) { return; }
        switch ((ParamStringId) id) {
            case ParamStringId::Amp:
                amp[string] = value;
                break;
            case ParamStringId::PickupPos1:
                pickupPos[string][0] = value;
                break;
            case ParamStringId::PickupPos2:
                pickupPos[string][1] = value;
                break;
            case ParamStringId::ExcitePos:
                excPos[string] = value;
                break;
            case ParamStringId::Beta:
                t[string].set_beta(value);
                break;
            case ParamStringId::Epsilon:
                t[string].set_epsilon(value);
                break;
            case ParamStringId::Rho:
                t[string].set_rho(value);
                break;
            case ParamStringId::Pluck:
                /// FIXME: might be nice to interpolate fractional exc. pos.
                this->pluck(string, (unsigned int) excPos[string], value);
                break;
            case ParamStringId::Count:
            default:;
                break;
        }
    }
};