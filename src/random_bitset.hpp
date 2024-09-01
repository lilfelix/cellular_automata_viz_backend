#ifndef RANDOM_BITSET_HPP
#define RANDOM_BITSET_HPP

#include <bitset>

// Define a 128-bit bitset
using Bitset128 = std::bitset<128>;

// Function to generate a random 128-bit bitset
Bitset128 generate_random_bitset128();

// Function to print the 128-bit bitset
void print_bitset128(const Bitset128& bitset);

// Optional: if you need to expose the test function
int test();

#endif // RANDOM_BITSET_HPP