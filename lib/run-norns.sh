#!/bin/bash

# NB: port 9999 is crone
#     port 1011 is matron's `remote` port, distinct from the system port 8888
/home/we/dust/code/ztsingou/ignore/ztsingou-engine/bin-rpi/ztsingou 9998 10111 &

# bleh. if we don't pause here, the client isn't running yet and jack_connect fails.
# guess it would be better if the client made the connection itself, before signalling ready. 
sleep 1

jack_connect ztsingou-engine:out1 crone:input_5
jack_connect ztsingou-engine:out2 crone:input_6
jack_connect crone:output_5 ztsingou-engine:in1
jack_connect crone:output_6 ztsingou-engine:in2