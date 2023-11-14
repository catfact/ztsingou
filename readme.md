# ztsingou

this is mostly a test / proof of concept for integrating external audio-processing engines with a norns script.


## engine structure

the "engine" in this case is a standalone JACK client and OSC server:
- engine source is included here under `ignore/ztsingou-engine`, and is a standard CMake project. 

- an executable binary built on rpi4 is included in `ignore/ztsingou-engine/bin-rpi`.

- the executable is launched, ports assigned, and JACK connections made in the script `lib/run-norns.sh`

- lua API for the engine is defined in `lib/ztsingou-engine.lua`. notably, it requires a callback to wait for the engine to be initialized.

## engine usage

the engine is a dual physical model of a string with nonlinear force characteristics. each string can be "plucked" or excited with external audio. 

per-string parameters:

```
-- output amplitude
'amp', 

-- pickup and excitation positions (2-15)
'pickupPos1', 'pickupPos2', 'excitePos', 

-- tension coefficient
'beta',

-- nonlinear stiffness coefficient
'epsilon', 

-- damping
'rho', 

-- stateless parameter; sets the position of the mass at the excitation point
'pluck'
```


global paramaeters:

```
-- amount of stereo spread between each pickup point on each string
-- first pickup on each string is always panned hard left/right
'spread', 

-- increase to "mono-ize" the final output mix
'mono', 

-- input gain for excitation
-- applies to both channels; each input channel goes to a different string
'gain',

-- iterations per sample. this is rounded to an integer. 
-- increasing IPS multiplies both pitch and CPU
'ips', 

-- number of masses to compute per string
-- changes the distribution of resonant modes,
-- increases / decreases CPU load
'masses'
```
