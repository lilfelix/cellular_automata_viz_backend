#include "random_bitset.hpp"

#include <iostream>
#include <bitset>
#include <random>
#include <ctime>

// Define a 128-bit bitset
using Bitset128 = std::bitset<128>;

// Function to generate a random 128-bit bitset
Bitset128 generate_random_bitset128() {
    std::random_device rd;   // Non-deterministic random number generator
    std::mt19937_64 gen(rd());  // 64-bit Mersenne Twister generator, seeded with random_device
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
    std::cout << "Random 128-bit Bitset: " << bitset << std::endl;
}

int test()
{
    // Generate a random 128-bit bitset
    Bitset128 random_bitset = generate_random_bitset128();

    // Print the bitset
    print_bitset128(random_bitset);

    return 0;
}