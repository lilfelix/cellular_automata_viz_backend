#include "random_bitset.hpp"

#include <iostream>
#include <random>
#include <ctime>
#include <vector>

bool is_middle_bit_set(uint8_t config)
{
    // Implement this function to return whether the middle bit of the configuration is set
    // Placeholder implementation:
    return config & 0b00001000; // Assuming the middle bit in a 3-bit configuration is the 2nd bit
}

// Function to determine if the central cell lives or dies in the next step
// given a rule encoded in a 128-bit bitset and 3D configurations (x, y, z).
bool does_cell_live(const Bitset128 &rule, uint8_t x, uint8_t y, uint8_t z) {
    // Combine the X, Y, Z configurations into a single index.
    // X occupies the most significant 3 bits (shifted by 6).
    // Y occupies the middle 3 bits (shifted by 3).
    // Z occupies the least significant 3 bits.
    //
    // This creates a unique 9-bit index ranging from 0 to 511
    // corresponding to all possible configurations of (x, y, z).
    uint16_t combined_configuration = (x << 6) | (y << 3) | z;
    
    // Calculate the index in the rule bitset by taking the lower 7 bits of the combined configuration.
    // The rule bitset is 128 bits wide, so we mod by 128 to get a valid index.
    uint16_t index_in_rule = combined_configuration % 128;
    
    // Return true if the bit at index_in_rule is set (i.e., the cell lives).
    // Otherwise, return false (i.e., the cell dies).
    return rule[index_in_rule];
}

std::vector<uint8_t> VALID_CONFIGURATIONS = {0, 1, 2, 3, 4, 5, 6, 7}; // Replace with actual valid configurations

// Function to generate and return the 3D mapping based on the given rule
ConfigMap generate_3d_mapping(const Bitset128 &rule)
{
    ConfigMap result_map;
    int bit_index = 0;

    for (uint8_t x : VALID_CONFIGURATIONS)
    {
        bool x_middle_bit_set = is_middle_bit_set(x);

        for (uint8_t y : VALID_CONFIGURATIONS)
        {
            if (is_middle_bit_set(y) == x_middle_bit_set)
            {
                for (uint8_t z : VALID_CONFIGURATIONS)
                {
                    if (is_middle_bit_set(z) == x_middle_bit_set)
                    {
                        bool cell_lives = does_cell_live(rule, x, y, z);
                        result_map[{x, y, z}] = cell_lives;
                        std::cout << "Generating 3D Moore neighbordhood. Bit index " + std::to_string(bit_index) + " : " + std::to_string(cell_lives) << std::endl;
                        ;
                        ++bit_index;
                    }
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