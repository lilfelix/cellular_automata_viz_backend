#include <iostream>
#include <vector>
#include <random>
#include "random_bitset.hpp"

using Grid3D = std::vector<std::vector<std::vector<uint8_t>>>;

// Function to generate the initial world state with random values (0 or 1)
Grid3D generate_initial_world_state(size_t x_max, size_t y_max, size_t z_max); 

// Function to update the world state based on the current state and rule map
Grid3D update_world_state(const Grid3D &current_world_state, const Bitset128 &rule);

// Function to print the XY slices of the 3D grid for each Z value
void print_slices(const Grid3D &world_state);