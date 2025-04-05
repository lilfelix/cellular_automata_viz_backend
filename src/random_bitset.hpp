#ifndef RANDOM_BITSET_HPP
#define RANDOM_BITSET_HPP

#include <bitset>
#include <unordered_map>
#include <tuple>
#include <cstdint> // fixed-width integer types like uint8_t and uint64_t,

using Bitset128 = std::bitset<128>;

// Function declarations
Bitset128 generate_random_bitset128();

void print_3d_mapping(const Bitset128 &rule);

// Function to determine if the central cell lives or dies in the next step
// given a rule encoded in a 128-bit bitset and 3D configurations (x, y, z).
bool does_cell_live(const Bitset128 &rule, uint8_t central_bit, uint8_t x, uint8_t y, uint8_t z);

/**
 * Accepts a uint64_t (max 64 bits set).
 * Sets the corresponding bits in a std::bitset<128>.
 * Higher bits are implicitly 0.
 * */
Bitset128 ParseBitSetRuleFromInteger(uint64_t rule_number);

// See Elementary Cellular Automata: https://content.wolfram.com/sites/13/2019/06/28-2-4.pdf
Bitset128 build_from_eca(uint8_t eca_rule_number);

#endif // RANDOM_BITSET_HPP