//
// Created by Alberto Santini on 23/06/2018.
//

#ifndef MOCK_DISTANCES_SERVER_H
#define MOCK_DISTANCES_SERVER_H

#include <stdint.h>
#include <stdlib.h>

// Serves the distances via a JSON HTTP API
void serve_distances(uint16_t port, size_t num_locations, char** locations, float* distances);

#endif //MOCK_DISTANCES_SERVER_H
