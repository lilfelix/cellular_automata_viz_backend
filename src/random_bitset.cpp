#include "random_bitset.hpp"

#include <iostream>
#include <random>
#include <ctime>
#include <vector>

// Constants for the number of possible configurations for a 2-bit sequence
constexpr size_t NUM_BIT_MAPS = 4;
// Array representing all valid 2-bit configurations
constexpr std::array<uint8_t, NUM_BIT_MAPS> VALID_BIT_MAPS = {
    0b00, 0b01, 0b10, 0b11};

// Function to determine if the central cell lives or dies in the next step
// given a rule encoded in a 128-bit bitset and 3D configurations (x, y, z).
bool does_cell_live(const Bitset128 &rule, uint8_t central_bit, uint8_t x, uint8_t y, uint8_t z)
{
    // Combine the central bit and the X, Y, Z configurations into a 7-bit index.
    // Central bit occupies the most significant bit
    // X, Y, Z each occupy 2 bits
    uint8_t combined_configuration = (central_bit << 6) | (x << 4) | (y << 2) | z;

    // Calculate the index in the rule bitset using the combined configuration.
    std::size_t index_in_rule = static_cast<std::size_t>(combined_configuration);

    // Return true if the bit at index_in_rule is set (i.e., the cell lives).
    // Otherwise, return false (i.e., the cell dies).
    return rule[index_in_rule];
}

// Function to generate and return the 3D mapping based on the given rule
void print_3d_mapping(const Bitset128 &rule)
{
    int bit_index = 0;

    for (uint8_t central_bit : {0, 1})
    {
        for (uint8_t x : VALID_BIT_MAPS)
        {
            for (uint8_t y : VALID_BIT_MAPS)
            {
                for (uint8_t z : VALID_BIT_MAPS)
                {
                    bool cell_lives = does_cell_live(rule, central_bit, x, y, z);
                    std::cout << "Generating 3D Moore neighborhood. Bit index "
                              << bit_index++ << " : " << (cell_lives ? "Lives" : "Dies") << std::endl;
                }
            }
        }
    }
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

/**
 * Accepts a uint64_t (max 64 bits set).
 * Sets the corresponding bits in a std::bitset<128>.
 * Higher bits are implicitly 0.
 * */
Bitset128 ParseBitSetRuleFromInteger(uint64_t rule_number)
{
    Bitset128 rule;
    for (int i = 0; i < 64; ++i)
    {
        if (rule_number & (1ULL << i))
        {
            rule.set(i);
        }
    }
    return rule;
}

Bitset128 ParseBitSetRuleFromString(const std::string &rule_str)
{
    Bitset128 rule;

    if (rule_str.size() != 16)
    {
        std::cerr << "Expected 16 bytes for 128-bit rule, got " << rule_str.size() << std::endl;
        return rule;
    }

    for (size_t bit = 0; bit < 128; ++bit)
    {
        size_t byte_idx = bit / 8;
        uint8_t byte = static_cast<uint8_t>(rule_str[byte_idx]);
        if (byte & (1 << (bit % 8)))
        {
            rule.set(bit);
        }
    }
    return rule;
}

// See Elementary Cellular Automata: https://content.wolfram.com/sites/13/2019/06/28-2-4.pdf
Bitset128 build_from_eca(uint8_t eca_rule_number)
{
    Bitset128 rule;
    for (uint8_t i = 0; i < 8; ++i)
    {
        uint8_t left = (i >> 2) & 1;
        uint8_t center = (i >> 1) & 1;
        uint8_t right = i & 1;

        uint8_t x_neighbors = (left << 1) | right;
        uint8_t y_neighbors = 0; // y_max = 1 → neighbors wrap to self
        uint8_t z_neighbors = 0;

        uint8_t index = (center << 6) | (x_neighbors << 4) | (y_neighbors << 2) | z_neighbors;

        if ((eca_rule_number >> i) & 1)
        {
            rule.set(index);
        }
    }
    return rule;
}