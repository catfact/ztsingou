#include <cstdlib>

#include <jack/jack.h>
#include <lo/lo.h>

#include "synth.hpp"

Synth synth;


jack_port_t* inPort[2];
jack_port_t* outPort[2];

int process(jack_nframes_t nframes, void *data) {

    const float* in[2];
    float* out[2];
    in[0] = (jack_default_audio_sample_t *) jack_port_get_buffer(inPort[0], nframes);
    in[1] = (jack_default_audio_sample_t *) jack_port_get_buffer(inPort[1], nframes);
    out[0] = (jack_default_audio_sample_t *) jack_port_get_buffer(outPort[0], nframes);
    out[1] = (jack_default_audio_sample_t *) jack_port_get_buffer(outPort[1], nframes);

    auto s = static_cast<Synth*>(data);
    s->processAudioBlockNonInterleaved(in, out, nframes);

    return 0;
}

int handle_param(const char *path, const char *types, lo_arg **argv, int argc,
			     lo_message data, void *user_data) {
    auto string = (unsigned int)argv[0]->i;
    auto id = argv[1]->i;
    auto val = argv[2]->f;
    synth.setParam(string, id, val);
}

int main(int argc, char *argv[]) { 
    const char* port = "99999";
    if (argc > 1) { 
        port = argv[1];
    }
    // FIXME: add error handler!
    lo_server_thread server = lo_server_thread_new(port, NULL);
    lo_server_thread_add_method(server, "/param", "iif", handle_param, NULL);



    return 0;
}