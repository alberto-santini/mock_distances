//
// Created by Alberto Santini on 23/06/2018.
//

#include "server.h"
#include "distances.h"

#include <event.h>
#include <evhttp.h>
#include <jansson.h>
#include <string.h>
#include <inttypes.h>

#define MAX_HEADER_SZ 1024

struct callback_args {
    struct event_base* base;
    size_t num_locations;
    char** locations;
    float* distances;
};

static void send_success_response(struct evhttp_request* request, char* origin, char* destination, float distance) {
    json_t* response = json_object();
    json_t* j_origin = json_string(origin);
    json_t* j_destination = json_string(destination);
    json_t* j_distance = json_real(distance);

    json_object_set_new(response, "origin", j_origin);
    json_object_set_new(response, "destination", j_destination);
    json_object_set_new(response, "distance", j_distance);

    const char* json_payload = json_dumps(response, JSON_COMPACT | JSON_REAL_PRECISION(4));
    size_t json_payload_len = strlen(json_payload);
    char response_header[MAX_HEADER_SZ];

    snprintf(response_header, MAX_HEADER_SZ, "%zu", json_payload_len);

    json_decref(response);

    evhttp_add_header(request->output_headers, "Content-Type", "application/json");
    evhttp_add_header(request->output_headers, "Content-Length", response_header);

    struct evbuffer* response_buffer = evbuffer_new();

    evbuffer_add(response_buffer, json_payload, json_payload_len);
    evhttp_send_reply(request, 200, "OK", response_buffer);
    evbuffer_free(response_buffer);
}

static void send_error_response(struct evhttp_request* request, char* text) {
    const size_t response_len = strlen(text);
    char response_header[MAX_HEADER_SZ];

    snprintf(response_header, MAX_HEADER_SZ, "%zu", response_len);

    evhttp_add_header(request->output_headers, "Content-Type", "text/plain");
    evhttp_add_header(request->output_headers, "Content-Length", response_header);

    struct evbuffer* response_buffer = evbuffer_new();

    evbuffer_add(response_buffer, text, response_len);
    evhttp_send_reply(request, 400, text, response_buffer);
    evbuffer_free(response_buffer);
}

static int get_origin_destination_from_json(json_t* js, char* origin, char* destination) {
    if(!json_is_object(js)) { return -1; }

    const json_t *const orig_node = json_object_get(js, "origin");
    if(!json_is_string(orig_node)) { return -2; }

    const json_t *const dest_node = json_object_get(js, "destination");
    if(!json_is_string(dest_node)) { return -3; }

    strncpy(origin, json_string_value(orig_node), MAX_LOCATION_NAME_SZ);

    printf("Got origin from Json request: %s\n", origin);

    strncpy(destination, json_string_value(dest_node), MAX_LOCATION_NAME_SZ);

    printf("Got destination from Json request: %s\n", destination);

    return 0;
}

static void json_request_handler(struct evhttp_request* request, void* arg) {
    struct callback_args* args = (struct callback_args*) arg;
    struct evbuffer* request_buffer = evhttp_request_get_input_buffer(request);
    const size_t request_len = evbuffer_get_length(request_buffer);
    char* request_data = (char*) malloc(request_len * sizeof(*request_data));

    if(!request_data) {
        fprintf(stderr, "Cannot allocate memory to read the request!\n");
        exit(EXIT_FAILURE);
    }

    memset(request_data, 0, request_len);
    evbuffer_copyout(request_buffer, request_data, request_len);

    printf("Got request: %s\n", request_data);

    json_error_t json_error;
    json_t* json_request = json_loadb(request_data, request_len, 0, &json_error);

    free(request_data);

    if(!json_request) {
        printf("Json load error on line %d: %s\n", json_error.line, json_error.text);
        send_error_response(request, "Bad JSON");
        return;
    }

    char origin[MAX_LOCATION_NAME_SZ];
    char destination[MAX_LOCATION_NAME_SZ];

    int success = get_origin_destination_from_json(json_request, origin, destination);

    json_decref(json_request);

    if(success != 0) {
        printf("Could not extract origin and destination from Json");
        send_error_response(request, "Malformed request");
        return;
    }

    printf("Parsed origin (%s) and destination (%s) from Json\n", origin, destination);

    float distance = get_distance(origin, destination, args->num_locations, args->locations, args->distances);

    if(distance < 0) {
        printf("Could not retrieve the distance between %s and %s\n", origin, destination);
        send_error_response(request, "Invalid origin/destination pair");
        return;
    }

    printf("Got distance between %s and %s: %.2f\n", origin, destination, distance);

    send_success_response(request, origin, destination, distance);
}

void serve_distances(uint16_t port, size_t num_locations, char** locations, float* distances) {
    struct callback_args* args = malloc(sizeof(*args));

    if(!args) {
        fprintf(stderr, "Cannot allocate memory for the net callback!\n");
        exit(EXIT_FAILURE);
    }

    args->base = event_base_new();
    args->num_locations = num_locations;
    args->locations = locations;
    args->distances = distances;

    struct evhttp* server = evhttp_new(args->base);

    if(!server) {
        fprintf(stderr, "Cannot start HTTP server!\n");
        exit(EXIT_FAILURE);
    }

    struct evhttp_bound_socket* handle = evhttp_bind_socket_with_handle(server, "0.0.0.0", port);

    if(!handle) {
        fprintf(stderr, "Cannot bind to port %"PRIu16"\n", port);
        exit(EXIT_FAILURE);
    }

    evhttp_set_cb(server, "/distance", json_request_handler, (void*) args);

    event_base_dispatch(args->base);
    evhttp_free(server);
    event_base_free(args->base);
}