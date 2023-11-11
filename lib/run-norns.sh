#!/bin/bash
/home/we/dust/code/ztsingou/ignore/ztsingou-engine/bin-rpi/ztsingou 9998 8888 &

sleep 1 # bleh

jack_connect ztsingou-engine:out1 crone:input_5
jack_connect ztsingou-engine:out2 crone:input_6
jack_connect crone:output_5 ztsingou-engine:in1
jack_connect crone:output_6 ztsingou-engine:in2