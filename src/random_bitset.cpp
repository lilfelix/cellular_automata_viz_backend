#include "random_bitset.hpp"

#include <iostream>
#include <random>
#include <ctime>
#include <vector>

// Function to determine if the central cell lives or dies in the next step
// given a rule encoded in a 128-bit bitset and 3D configurations (x, y, z).
bool does_cell_live(const Bitset128 &rule, uint8_t central_bit, uint8_t x, uint8_t y, uint8_t z) {
    // Combine the central bit and the X, Y, Z configurations into a 7-bit index.
    // Central bit occupies the most significant bit
    // X, Y, Z each occupy 2 bits
    uint8_t combined_configuration = (central_bit << 6) | (x << 4) | (y << 2) | z;
    
    // Calculate the index in the rule bitset using the combined configuration.
    uint8_t index_in_rule = combined_configuration % 128;
    
    // Return true if the bit at index_in_rule is set (i.e., the cell lives).
    // Otherwise, return false (i.e., the cell dies).
    return rule[index_in_rule];
}

// Function to generate and return the 3D mapping based on the given rule
ConfigMap generate_3d_mapping(const Bitset128 &rule) {
    ConfigMap result_map;
    int bit_index = 0;

    for (uint8_t central_bit : {0, 1}) {
        for (uint8_t x : {0, 1, 2, 3}) {
            for (uint8_t y : {0, 1, 2, 3}) {
                for (uint8_t z : {0, 1, 2, 3}) {
                    bool cell_lives = does_cell_live(rule, central_bit, x, y, z);
                    result_map[{x, y, z}] = cell_lives;
                    std::cout << "Generating 3D Moore neighborhood. Bit index " 
                              << bit_index++ << " : " << (cell_lives ? "Lives" : "Dies") << std::endl;
                }
            }
        }
    }

    return result_map;
}

// Function to generate a random 128-bit bitset
Bitset128 generate_random_bitset128()
{
    std::random_device rd;     // Non-deterministic random number generator
    std::mt19937_64 gen(rd()); // 64-bit Mersenne Twister generator, seeded with random_device
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

    // Generate two 64-bit random numbers to fill the 128-bit bitset
    uint64_t lower = dist(gen);
    uint64_t upper = dist(gen);

    // Convert the 64-bit integers to bitsets
    std::bitset<128> lower_bits(lower);
    std::bitset<128> upper_bits(upper);

    // Shift the upper bits by 64 and combine with lower bits
    Bitset128 random_bitset = (upper_bits << 64) | lower_bits;

    return random_bitset;
}

// Function to print the 128-bit bitset
void print_bitset128(const Bitset128 &bitset)
{
    std::cout << "Random 128-bit Bitset:\n" << bitset << std::endl;
}

int test()
{
    Bitset128 random_bitset = generate_random_bitset128();
    print_bitset128(random_bitset);
    generate_3d_mapping(random_bitset);

    return 0;
}