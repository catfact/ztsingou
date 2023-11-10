#include <cstdlib>


#include <jack/jack.h>
#include <lo/lo.h>

#include "synth.hpp"

Synth synth;

int process(jack_nframes_t nframes, void *arg) {
    // jack_default_audio_sample_t *in, *out;
    // in = (jack_default_audio_sample_t *) jack_port_get_buffer(input_port, nframes);
    // out = (jack_default_audio_sample_t *) jack_port_get_buffer(output_port, nframes);

    // for (int frame=0; frame<nframes; ++frame) { 

    // }

    return 0;
}

int handle_param(const char *path, const char *types, lo_arg **argv, int argc,
			     lo_message data, void *user_data) {

                 }

int main(int argc, char *argv[]) { 
    const char* port = "99999";
    if (argc > 1) { 
        port = argv[1];
    }
    // FIXME: add error handler!
    lo_server_thread server = lo_server_thread_new(port, NULL);
    lo_server_thread_add_method(server, "/param", "if", handle_param, NULL);
}