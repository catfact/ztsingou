#include <cstdlib>
#include <csignal>

#include <condition_variable>
#include <mutex>
#include <thread>

#include <jack/jack.h>
#include <lo/lo.h>

#include "synth.hpp"

Synth synth;
jack_client_t *client;
jack_port_t* inPort[2];
jack_port_t* outPort[2];

lo_server_thread server;

std::mutex m;
std::condition_variable cv;
bool exiting = false;

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
    auto id = argv[0]->i;
    auto string = (unsigned int)argv[1]->i;
    auto val = argv[2]->f;
    synth.setParam(string, id, val);
    return 0;
}


int handle_quit(const char *path, const char *types, lo_arg **argv, int argc,
                 lo_message data, void *user_data) {
    
    {
        std::lock_guard<std::mutex> lk(m);
        exiting = true;
    }
    cv.notify_one();
    return 0;
}

void cleanup() {
    // FIXME: cleanup jack client probably
    lo_server_thread_stop(server);
}

void signal_handler(int sig) {
    cleanup();
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGABRT, signal_handler);

    const char* rxPort = "9999";
    if (argc > 1) { 
        rxPort = argv[1];
    }
    const char* txPort = "9999";
    if (argc > 2) { 
        txPort = argv[1];
    }

    ///---------------------------------
    ///--- setup jack ---
    // create jack client
    client = jack_client_open("ztsingou-engine", JackNullOption, NULL);
    if (client == nullptr) {
        fprintf(stderr, "jack server not running?\n");
        return 1;
    }

    // register jack ports
    inPort[0] = jack_port_register(client, "in1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    inPort[1] = jack_port_register(client, "in2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    outPort[0] = jack_port_register(client, "out1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    outPort[1] = jack_port_register(client, "out2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    // assign jack callback
    jack_set_process_callback(client, process, &synth);

    // start the jack client
    if (jack_activate(client)) {
        fprintf(stderr, "cannot activate client");
        return 1;
    }

    ///---------------------------------
    ///--- setup OSC server ---

    // FIXME: should handle liblo server errors
    server = lo_server_thread_new(rxPort, NULL);
    lo_server_thread_add_method(server, "/param", "iif", handle_param, NULL);
    lo_server_thread_add_method(server, "/quit", "", handle_quit, NULL);
    lo_server_thread_start(server);

    /// ... do any other init work here ...

    lo_address tx = lo_address_new("localhost", txPort);
    lo_send(tx, "/ready", "");

    ///---------------------------------
    ///------ block until quit ---------
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, []{ return exiting; });
    cleanup();
    return 0;
}