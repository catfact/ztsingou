#!/bin/bash
./bin-macos/ztsingou 9998 8888 & 

jack_connect ztsingou-engine:out1 system:playback_1
jack_connect ztsingou-engine:out2 system:playback_2
jack_connect system:capture_1 ztsingou-engine:in1
jack_connect system:capture_2 ztsingou-engine:in2