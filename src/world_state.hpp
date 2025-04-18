#include <iostream>
#include <random>
#include <map>
#include <tuple>
#include <tl/expected.hpp>
#include "bit_packed_grid_3d.hpp"
#include "random_bitset.hpp"

class WorldStateContainer
{
public:
    uint64_t next_world_state_id;
    std::map<uint64_t, BitPackedGrid3D> world_states;
    RuleMode rule_mode;

    WorldStateContainer();
    tl::expected<std::tuple<uint64_t, BitPackedGrid3D>, std::string> InitWorldState1D(size_t x_max, size_t y_max, size_t z_max);
    tl::expected<std::tuple<uint64_t, BitPackedGrid3D>, std::string> InitWorldState3D(size_t x_max, size_t y_max, size_t z_max);
    // Generate the initial world state with random values (0 or 1)
    tl::expected<std::tuple<uint64_t, BitPackedGrid3D>, std::string> InitWorldStateRandom(size_t x_max, size_t y_max, size_t z_max);
    // Update the world state based on the current state and rule map
    BitPackedGrid3D UpdateWorldState(const BitPackedGrid3D &current_world_state, const Bitset128 &rule);
    // Print the XY slices of the 3D grid for each Z value
    void PrintSlices(const BitPackedGrid3D &world_state);
    // Check if two states are
    bool IsSameAs(const BitPackedGrid3D &state_a, const BitPackedGrid3D &state_b);
};