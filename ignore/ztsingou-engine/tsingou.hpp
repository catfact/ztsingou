#pragma once

/*
  fermi-pasta-ulam-tsingou resonator

  this is a "physical" model of a nonlinear string.

  the string is a number of connected masses

  the force acting on a given mass is a function of its displacement.
  this function has a linear term and a variable nonlinear (cubic) term

  d1 = upper displacement
  d0 = lower displacement

  f = b* [ (d1 - d0) + e*(d1^3 - d0^3) ]
*/

class Tsingou {
public:
    static constexpr int NUM_MASSES = 8;


    double get_output(double pos) {
        int pi0 = (int) pos;
        double pf = pos - (double) pi0;
        pi0 = pi0 % (NUM_MASSES);
        int pi1 = (pi0 + 1) % NUM_MASSES;
        return x[pi0] + pf * (x[pi1] - x[pi0]);
    }

    void update(double input) {
        for (int i = 0; i < ips; i++) {
            update_f();
            update_v();
            update_x();
        }
    }

    void set_pos(int pos, double val) {
        if (pos > 1 && pos < (NUM_MASSES - 2)) {
            if (val > 1.0) { val = 1.0; }
            if (val < -1.0) { val = -1.0; }
            x[pos] = val;
        }
    }

    void set_beta(double val) {
        beta = val;
    }

    void set_epsilon(double val) {
        epsilon = val;
    }

    void set_rho(double val) {
        rho = val;
    }

private:
    //--- synthesis parameters
    // sample rate
    float sr;
    // iterations per sample
    int ips;

    //----- model parameters
    // "physical" time step (in seconds)
    double dt;
    // tension parameter (controls pitch)
    double beta;
    // restoring force (controls duration)
    double rho;
    // nonlinear parameter
    double epsilon;
    // array of forces
    double f[NUM_MASSES];
    // array of velocities
    double v[NUM_MASSES];
    // array of positions
    double x[NUM_MASSES];

    // update forces
    void update_f() {
        // start and end positions are fixed at zero
        double fx;
        //double *x = x;
        double d0, d1;
        for (int i = 1; i < NUM_MASSES - 1; i++) {
            d1 = x[i + 1] - x[i];
            d0 = x[i] - x[i - 1];
            fx = d1 - d0 + epsilon * ((d1 * d1 * d1) - (d0 * d0 * d0));
            fx *= beta;
            fx -= rho * v[i];
            f[i] = fx;
        }
    }

    // update velocities
    void update_v() {
        // start and end positions are fixed at zero
        for (int i = 1; i < NUM_MASSES - 1; i++) {
            v[i] += f[i] * dt;
        }
    }

    // update positions
    void update_x() {
        // start and end positions are fixed at zero
        for (int i = 1; i < NUM_MASSES - 1; i++) {
            x[i] += v[i];
        }
    }

    //---------------------
    //---- extern functions
public:
    void init(double aDt, float aSr) {
        dt = aDt;
        sr = aSr;
        clear_state();
    }

    void clear_state() {
        for (int i = 0; i < NUM_MASSES; i++) {
            x[i] = 0.0;
            v[i] = 0.0;
            f[i] = 0.0;
        }
    }

    void set_ips(int val) { ips = val; }
};