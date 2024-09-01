#include "world_state.hpp"


// Function to generate the initial world state with random values (0 or 1)
Grid3D generate_initial_world_state(size_t x_max, size_t y_max, size_t z_max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist(0, 1);

    Grid3D world_state(x_max, std::vector<std::vector<uint8_t>>(y_max, std::vector<uint8_t>(z_max)));

    for (size_t x = 0; x < x_max; ++x) {
        for (size_t y = 0; y < y_max; ++y) {
            for (size_t z = 0; z < z_max; ++z) {
                world_state[x][y][z] = dist(gen);
            }
        }
    }

    return world_state;
}

// Function to update the world state based on the current state and rule map
Grid3D update_world_state(const Grid3D &current_world_state, const ConfigMap &rule_map) {
    size_t x_max = current_world_state.size();
    size_t y_max = current_world_state[0].size();
    size_t z_max = current_world_state[0][0].size();

    // Create a new grid for the next state. This is a deep copy of the current world state.
    Grid3D next_world_state = current_world_state;

    for (size_t x = 1; x < x_max - 1; ++x) {
        for (size_t y = 1; y < y_max - 1; ++y) {
            for (size_t z = 1; z < z_max - 1; ++z) {
                uint8_t central_bit = current_world_state[x][y][z];
                uint8_t x_neighbors = (current_world_state[x - 1][y][z] << 1) | current_world_state[x + 1][y][z];
                uint8_t y_neighbors = (current_world_state[x][y - 1][z] << 1) | current_world_state[x][y + 1][z];
                uint8_t z_neighbors = (current_world_state[x][y][z - 1] << 1) | current_world_state[x][y][z + 1];

                std::tuple<uint8_t, uint8_t, uint8_t> config = {x_neighbors, y_neighbors, z_neighbors};
                
                // Use the rule map to determine the new state based on the central bit and neighbor configuration
                next_world_state[x][y][z] = rule_map.at(config) ? central_bit : 0;
            }
        }
    }

    return next_world_state;
}

// Function to print the XY slices of the 3D grid for each Z value in a compact format
void print_slices(const Grid3D &world_state) {
    size_t x_max = world_state.size();
    size_t y_max = world_state[0].size();
    size_t z_max = world_state[0][0].size();

    for (size_t y = 0; y < y_max; ++y) {
        for (size_t z = 0; z < z_max; ++z) {
            for (size_t x = 0; x < x_max; ++x) {
                std::cout << static_cast<int>(world_state[x][y][z]) << " ";
            }
            if (z < z_max - 1) {
                std::cout << ",\t";  // Separate slices horizontally with a comma and tab
            }
        }
        std::cout << "\n";  // Move to the next row of the XY plane
    }
    std::cout << std::endl;  // Final newline for clarity
}