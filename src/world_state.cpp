#include "world_state.hpp"
#include "random_bitset.hpp"
#include <sstream>

WorldStateContainer::WorldStateContainer() : next_world_state_id(0), world_states() {}

/**
 * Function to generate the initial world state based on Wolfram's rule-naming scheme
 * - The neighborhood of a cell can include itself, but doesn't have to
 * - For a 1D grid with 3 neighbors (including itself), there are 2^3=8 neighborhood states.
 *   Having two states {0,1} means there are 2^8 possible rules
 */
tl::expected<std::tuple<uint64_t, BitPackedGrid3D>, std::string> WorldStateContainer::InitWorldState1D(size_t x_max, size_t y_max, size_t z_max)
{
    rule_mode = RuleMode::RULE_1D_ECA;
    // By convention the x-axis is used to determine where to place the "central dot"
    const size_t central_dot_idx = x_max / 2;
    if (central_dot_idx < 2 || y_max < 1 || z_max < 1)
    {
        std::ostringstream oss;
        oss << "Invalid dimensions: x=" << x_max << ", y=" << y_max << ", z=" << z_max << ", center=" << central_dot_idx;
        return tl::unexpected(oss.str());
    }

    BitPackedGrid3D world_state = BitPackedGrid3D(x_max, y_max, z_max);
    // For even-sized dimensions, this chooses the lower of the two central indices.
    size_t cx = x_max / 2;
    size_t cy = y_max / 2;
    size_t cz = z_max / 2;

    world_state.set(cx, cy, cz, true);
    uint64_t world_state_id = next_world_state_id++;
    return tl::expected<std::tuple<uint64_t, BitPackedGrid3D>, std::string>{
        std::make_tuple(world_state_id, std::move(world_state))};
}

tl::expected<std::tuple<uint64_t, BitPackedGrid3D>, std::string> WorldStateContainer::InitWorldState3D(size_t x_max, size_t y_max, size_t z_max)
{
    rule_mode = RuleMode::RULE_3D;
    if (x_max / 2 < 2 || y_max / 2 < 2 || z_max / 2 < 2)
    {
        std::ostringstream oss;
        oss << "Invalid dimensions: x=" << x_max << ", y=" << y_max << ", z=" << z_max;
        return tl::unexpected(oss.str());
    }
    BitPackedGrid3D world_state = BitPackedGrid3D(x_max, y_max, z_max);
    // For even-sized dimensions, this chooses the lower of the two central indices.
    size_t cx = x_max / 2;
    size_t cy = y_max / 2;
    size_t cz = z_max / 2;

    world_state.set(cx, cy, cz, true);
    uint64_t world_state_id = next_world_state_id++;
    return tl::expected<std::tuple<uint64_t, BitPackedGrid3D>, std::string>{
        std::make_tuple(world_state_id, std::move(world_state))};
}

/**
 * Function to generate the initial world state with random values (0 or 1)
 */
tl::expected<std::tuple<uint64_t, BitPackedGrid3D>, std::string> WorldStateContainer::InitWorldStateRandom(size_t x_max, size_t y_max, size_t z_max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist(0, 1);

    BitPackedGrid3D world_state(x_max, y_max, z_max);

    for (size_t i = 0; i < x_max * y_max * z_max; ++i)
    {
        world_state.set(i, dist(gen));
    }
    uint64_t world_state_id = next_world_state_id++;
    return tl::expected<std::tuple<uint64_t, BitPackedGrid3D>, std::string>{
        std::make_tuple(world_state_id, std::move(world_state))};
}

// Function to update the world state based on the current state and rule map
// The grid is toroidal(wraps around in all directions)
BitPackedGrid3D WorldStateContainer::UpdateWorldState(
    const BitPackedGrid3D &current_world_state,
    const Bitset128 &rule)
{
    // Create a new grid for the next state. This is a deep copy of the current world state.
    BitPackedGrid3D next_world_state = current_world_state;
    const size_t x_max = current_world_state.x_max;
    const size_t y_max = current_world_state.y_max;
    const size_t z_max = current_world_state.z_max;

    // Iterate over all cells including boundaries
    for (size_t i = 0; i < x_max * y_max * z_max; ++i)
    {
        auto [x, y, z] = current_world_state.unpack_bit_index(i);
        uint8_t central_bit = current_world_state.get(i);
        // Get the neighbors, handling boundary conditions with modulo
        // (x + x_max - 1) % x_max safely wraps around to x_max - 1 when x == 0
        // Below, two binary neighbor values are packed into a 2-bit value, like this:
        // uint8_t pair = (left << 1) | right;
        uint8_t x_neighbors = (current_world_state.get((x + x_max - 1) % x_max, y, z) << 1) |
                              current_world_state.get((x + 1) % x_max, y, z);
        uint8_t y_neighbors = (current_world_state.get(x, (y + y_max - 1) % y_max, z) << 1) |
                              current_world_state.get(x, (y + 1) % y_max, z);
        uint8_t z_neighbors = (current_world_state.get(x, y, (z + z_max - 1) % z_max) << 1) |
                              current_world_state.get(x, y, (z + 1) % z_max);

        if (rule_mode == RULE_1D_ECA)
        {
            y_neighbors = 0;
            z_neighbors = 0;
        }

        bool cell_lives = does_cell_live(rule, central_bit, x_neighbors, y_neighbors, z_neighbors);

        // Apply the rule: update the cell state in the next_world_state based on the central bit
        next_world_state.set(i, cell_lives);
    }

    return next_world_state;
}

// Function to print the XY slices of the 3D grid for each Z value in a compact format
void WorldStateContainer::PrintSlices(const BitPackedGrid3D &world_state)
{
    const size_t x_max = world_state.x_max;
    const size_t y_max = world_state.y_max;
    const size_t z_max = world_state.z_max;

    for (size_t y = 0; y < y_max; ++y)
    {
        for (size_t z = 0; z < z_max; ++z)
        {
            for (size_t x = 0; x < x_max; ++x)
            {
                std::cout << static_cast<int>(world_state.get(x, y, z)) << " ";
            }
            if (z < z_max - 1)
            {
                std::cout << ",\t"; // Separate slices horizontally with a comma and tab
            }
        }
        std::cout << "\n"; // Move to the next row of the XY plane
    }
    std::cout << std::endl; // Final newline for clarity
}

bool WorldStateContainer::IsSameAs(const BitPackedGrid3D &state_a, const BitPackedGrid3D &state_b)
{
    return state_a == state_b;
}