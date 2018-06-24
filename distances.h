//
// Created by Alberto Santini on 23/06/2018.
//

#ifndef MOCK_DISTANCES_DISTANCES_H
#define MOCK_DISTANCES_DISTANCES_H

#define MAX_LOCATION_NAME_SZ 128

// Reads a file with the distances between locations.
// Format:
// First line: a number n of locations
// Next n lines: the names of the locations, one per line
// Next n-1 lines: an upper triangular matrix containing the distnaces (without the diagonal)
void read_distances(char* file, size_t* num_locations, char*** locations, float** distances);

// Returns the distance between the origin and the destination.
// If negative, there was an error.
float get_distance(char* origin, char* destination, size_t num_locations, char** locations, float* distances);

#endif //MOCK_DISTANCES_DISTANCES_H
