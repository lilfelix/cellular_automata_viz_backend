#ifndef RANDOM_BITSET_HPP
#define RANDOM_BITSET_HPP

#include <bitset>
#include <unordered_map>
#include <tuple>
#include <cstdint> // fixed-width integer types like uint8_t and uint64_t,

// Define a 128-bit bitset
using Bitset128 = std::bitset<128>;

// Custom hash function for std::tuple<uint8_t, uint8_t, uint8_t>
namespace std
{
    template <>
    struct hash<std::tuple<uint8_t, uint8_t, uint8_t>>
    {
        std::size_t operator()(const std::tuple<uint8_t, uint8_t, uint8_t> &t) const
        {
            std::size_t h1 = std::hash<uint8_t>()(std::get<0>(t));
            std::size_t h2 = std::hash<uint8_t>()(std::get<1>(t));
            std::size_t h3 = std::hash<uint8_t>()(std::get<2>(t));
            return h1 ^ (h2 << 1) ^ (h3 << 2); // Combine the hashes
        }
    };
}

using ConfigMap = std::unordered_map<std::tuple<uint8_t, uint8_t, uint8_t>, bool>;

// Function declarations
Bitset128 generate_random_bitset128();
ConfigMap generate_3d_mapping(const Bitset128 &rule);

#endif // RANDOM_BITSET_HPP