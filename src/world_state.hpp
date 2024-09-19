#include <iostream>
#include <vector>
#include <random>
#include <map>
#include <tuple>
#include "random_bitset.hpp"

using Grid3D = std::vector<std::vector<std::vector<uint8_t>>>;

class WorldStateContainer
{
public:
    uint64_t next_world_state_id;
    std::map<uint64_t, Grid3D> world_states;

    WorldStateContainer();
    // Generate the initial world state with random values (0 or 1)
    std::tuple<uint64_t, Grid3D> InitWorldState(size_t x_max, size_t y_max, size_t z_max);
    // Update the world state based on the current state and rule map
    Grid3D UpdateWorldState(const Grid3D &current_world_state, const Bitset128 &rule);
    // Print the XY slices of the 3D grid for each Z value
    void PrintSlices(const Grid3D &world_state);
};