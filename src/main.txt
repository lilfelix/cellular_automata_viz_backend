#include <iostream>
#include <bitset>
#include <array>
#include "random_bitset.hpp"
#include "world_state.hpp"



// Ticker function to move time forward, updating the world state at each step
Grid3D run_simulation(Grid3D &world_state, const Bitset128 &rule, size_t steps)
{
    for (size_t step = 0; step < steps; ++step)
    {
        world_state = update_world_state(world_state, rule);
        // if ((step % (steps / 10)) == 0)
        // {
        //     std::cout << "Completed step " << step << " of " << steps << std::endl;
        // }
        print_slices(world_state);
    }
    return world_state;
}

int old_main()
{
    // Parameters for the simulation
    size_t x_max = 3, y_max = 3, z_max = 3;
    size_t steps = 100;

    // Generate a random rule
    Bitset128 rule = generate_random_bitset128();

    std::cout << "Random 128-bit Bitset:\n"
              << rule << std::endl;

    // Generate the initial world state
    Grid3D world_state = generate_initial_world_state(x_max, y_max, z_max);
    print_slices(world_state);

    // Run the simulation
    auto final_world_state = run_simulation(world_state, rule, steps);

    std::cout << "Comparison with initial world state:\n"
              << std::endl;
    print_slices(final_world_state);

    return 0;
}