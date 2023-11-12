#include <cstdlib>
#include <csignal>

#include <condition_variable>
#include <iostream>
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

int handle_param_global(const char *path, const char *types, lo_arg **argv, int argc,
			     lo_message data, void *user_data) {
    auto id = argv[0]->i;
    auto val = argv[1]->f;
    std::cerr << "param global: " << id << ", " << val << std::endl;
    synth.setParamGlobal(id, val);
    return 0;
}

int handle_param_string(const char *path, const char *types, lo_arg **argv, int argc,
			     lo_message data, void *user_data) {
    auto id = argv[0]->i;
    auto string = (unsigned int)argv[1]->i;
    auto val = argv[2]->f;
    std::cerr << "param string: " << id << ", " << string << ", " << val << std::endl;
    synth.setParamString(string, id, val);
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
    const char* txPort = "8888";
    if (argc > 2) { 
        txPort = argv[2];
    }

    ///---------------------------------
    ///--- setup jack ---
    // create jack client
    fprintf(stderr, "[--zt--] creating JACK client...\n");
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
        fprintf(stderr, "cannot activate client\n");
        return 1;
    }

    ///---------------------------------
    ///--- setup OSC server ---

    // FIXME: should handle liblo server errors

    fprintf(stderr, "[--zt--] starting OSC server...\n");
    server = lo_server_thread_new(rxPort, NULL);
    lo_server_thread_add_method(server, "/param/string", "iif", handle_param_string, NULL);
    lo_server_thread_add_method(server, "/param/global", "if", handle_param_global, NULL);
    lo_server_thread_add_method(server, "/quit", "", handle_quit, NULL);
    lo_server_thread_start(server);

    /// ... do any other init work here ...
    fprintf(stderr, "[--zt--] sending ready message; port = %s\n", txPort);
    lo_address tx = lo_address_new("127.0.0.1", txPort);
    lo_send(tx, "/ztsingou/ready", "");

    ///---------------------------------
    ///------ block until quit ---------
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, []{ return exiting; });
    cleanup();
    return 0;
}