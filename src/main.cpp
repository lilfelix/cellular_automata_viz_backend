#include <iostream>
#include <bitset>
#include <array>
#include "random_bitset.hpp"
#include "world_state.hpp"

// Ticker function to move time forward, updating the world state at each step
void run_simulation(Grid3D &world_state, const ConfigMap &rule_map, size_t steps)
{
    for (size_t step = 0; step < steps; ++step)
    {
        world_state = update_world_state(world_state, rule_map);
        print_slices(world_state);
        std::cout << "Completed step " << step + 1 << " of " << steps << std::endl;
    }
}

int main()
{
    // Parameters for the simulation
    size_t x_max = 3, y_max = 3, z_max = 3;
    size_t steps = 10000;

    // Generate a random rule
    Bitset128 rule = generate_random_bitset128();

    std::cout << "Random 128-bit Bitset:\n"
              << rule << std::endl;

    // Generate the rule map based on the random rule
    ConfigMap rule_map = generate_3d_mapping(rule);

    // Generate the initial world state
    Grid3D world_state = generate_initial_world_state(x_max, y_max, z_max);
    Grid3D saved_initial_world_state = world_state;

    // Run the simulation
    run_simulation(world_state, rule_map, steps);

    std::cout << "Comparison with initial world state:\n" << std::endl;
    print_slices(saved_initial_world_state);

    return 0;
}