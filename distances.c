//
// Created by Alberto Santini on 23/06/2018.
//

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include <string.h>
#include "distances.h"

void read_distances(char* file, size_t* num_locations, char*** locations, float** distances) {
    FILE* fp = fopen(file, "r");

    uint32_t line_n = 0;
    char line_buffer[MAX_LOCATION_NAME_SZ];

    while(fgets(line_buffer, MAX_LOCATION_NAME_SZ, fp)) {
        ++line_n;

        if(line_n == 1) {
            assert(num_locations);
            sscanf(line_buffer, "%zu", num_locations);
            assert(*num_locations > 0);

            printf("Reading %zu locations from %s\n", *num_locations, file);

            *locations = malloc(*num_locations * sizeof(**locations));
            if(!*locations) { goto error; }

            for(size_t i = 0; i < *num_locations; ++i) {
                (*locations)[i] = malloc(MAX_LOCATION_NAME_SZ * sizeof(***locations));
                if(!(*locations)[i]) { goto error; }
            }

            *distances = malloc((*num_locations) * (*num_locations) * sizeof(**distances));
            if(!*distances) { goto error; }

        } else if(line_n >= 2 && line_n <= 1 + (*num_locations)) {
            const size_t location_id = line_n - 2;
            sscanf(line_buffer, "%s", (*locations)[location_id]);

        } else if(line_n >= 2 + *num_locations && line_n <= 2 * (*num_locations)) {
            const size_t origin_id = line_n - 2 - (*num_locations);
            const char* buffer_start = line_buffer;
            char space;

            assert(origin_id < *num_locations);

            for(size_t destination_id = origin_id + 1; destination_id < *num_locations; ++destination_id) {
                const size_t direct_distance_id = origin_id * (*num_locations) + destination_id;
                const size_t inverse_distance_id = destination_id * (*num_locations) + origin_id;
                int bytes_consumed;

                sscanf(buffer_start, "%f%c%n", &(*distances)[direct_distance_id], &space, &bytes_consumed);
                buffer_start += bytes_consumed;

                // Distances are symmetric
                (*distances)[inverse_distance_id] = (*distances)[direct_distance_id];
            }
        }
    }

    fclose(fp);

    printf("Read locations: ");
    for(size_t i = 0; i < *num_locations; ++i) {
        printf("%s ", (*locations)[i]);
    }
    printf("\n");

    return;

    error:
        fclose(fp);

        if(*locations) {
            free(*locations);
            *locations = NULL;
        }

        if(*distances) {
            free(*distances);
            *distances = NULL;
        }
}

float get_distance(char* origin, char* destination, size_t num_locations, char** locations, float* distances) {
    size_t orig_id = num_locations;
    size_t dest_id = num_locations;

    printf("Getting distance between %s and %s (out of %zu locations)\n", origin, destination, num_locations);

    for(size_t i = 0; i < num_locations; ++i) {
        if(orig_id == num_locations && strncmp(locations[i], origin, MAX_LOCATION_NAME_SZ) == 0) {
            orig_id = i;
        }

        if(dest_id == num_locations && strncmp(locations[i], destination, MAX_LOCATION_NAME_SZ) == 0) {
            dest_id = i;
        }

        if(orig_id < num_locations && dest_id < num_locations) {
            break;
        }
    }

    if(orig_id == num_locations) {
        printf("Could not find id for origin %s\n", origin);
        return -1;
    }

    if(dest_id == num_locations) {
        printf("Could not find id for destination %s\n", destination);
        return -1;
    }

    printf("The corresponding indices are %zu and %zu\n", orig_id, dest_id);

    return distances[orig_id * num_locations + dest_id];
}