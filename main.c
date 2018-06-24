#include <stdio.h>
#include <stdint.h>
#include <argp.h>
#include <stdlib.h>
#include "distances.h"
#include "server.h"

// Explanatory text for --help
static const char doc[] = "mock_distances -- reads distances between locations from file, and serves them via a simple network API";
// No free arguments allowed
static const char free_args[] = "";
// We expect two named arguments: -p and -f
static const struct argp_option options[] = {
        {"port", 'p', "PORT", 0, "The port to use to serve the API"},
        {"file", 'f', "FILE", 0, "The file containing the distances"},
        {0}
};
// The arguments tell us the port to listen to, and the distance file to read
struct arguments {
    uint16_t api_port;
    char* distances_file;
};

// Number of locations
static size_t num_locations;
// Array of locations (strings)
static char** locations;
// Distance matrix
static float* distances;

// Parses one command line option
static error_t parse_options(int key, char* arg, struct argp_state* state) {
    struct arguments* args = state->input;

    if(key == 'p') {
        unsigned long port_opt = strtoul(arg, NULL, 10);

        if(port_opt >= UINT16_MAX) {
            fprintf(stderr, "Invalid port number!\n");
            exit(EXIT_FAILURE);
        }
        args->api_port = (uint16_t) port_opt;
    } else if(key == 'f') {
        args->distances_file = arg;
    } else {
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

// Argp parser for command line options
static struct argp parser = { options, parse_options, free_args, doc };

int main(int argc, char** argv) {
    struct arguments args;

    // Default values for the arguments
    args.api_port = 5500;
    args.distances_file = "distances.txt";

    // Parse command line arguments
    argp_parse(&parser, argc, argv, 0, 0, &args);

    // Read the distance file
    read_distances(args.distances_file, &num_locations, &locations, &distances);

    // Serve the distance file via the net
    serve_distances(args.api_port, num_locations, locations, distances);

    return 0;
}