#include <iostream>
#include <bitset>
#include <array>
#include "random_bitset.hpp"

// Constants for the number of possible configurations for a 3-bit sequence
constexpr size_t NUM_CONFIGURATIONS = 8;
constexpr uint8_t MIDDLE_BIT_MASK = 0b010;

// Array representing all valid 3-bit configurations (000 to 111)
constexpr std::array<uint8_t, NUM_CONFIGURATIONS> VALID_CONFIGURATIONS = {
    0b000, 0b001, 0b010, 0b011,
    0b100, 0b101, 0b110, 0b111
};

// Utility function to check if the middle bit of a 3-bit configuration is set
inline bool is_middle_bit_set(uint8_t configuration) {
    return (configuration & MIDDLE_BIT_MASK) != 0;
}

// Function to determine if the central cell lives or dies in the next step
bool does_cell_live(uint8_t rule, uint8_t x, uint8_t y, uint8_t z) {
    // Combine the X, Y, Z configurations into a single 9-bit value
    // X occupies the most significant 3 bits (shifted by 6)
    // Y occupies the middle 3 bits (shifted by 3)
    // Z occupies the least significant 3 bits
    uint16_t combined_configuration = (x << 6) | (y << 3) | z;
    
    // Determine if the corresponding bit in the rule is set
    // Shift the rule's bits right to align with the combined configuration
    return (rule & (1 << (7 - (combined_configuration & 0b11111111)))) != 0;
}

// Function to print the mapping of a 3D vector configuration
void print_mapping(uint8_t x, uint8_t y, uint8_t z, int bit_index, bool cell_lives) {
    std::cout << "X: " << std::bitset<3>(x)
              << " Y: " << std::bitset<3>(y)
              << " Z: " << std::bitset<3>(z)
              << " -> Bit " << bit_index 
              << " : " << (cell_lives ? "Lives" : "Dies") << std::endl;
}

// Function to generate and display the 3D mapping based on the given rule
void generate_3d_mapping(uint8_t rule) {
    int bit_index = 127;
    for (uint8_t x : VALID_CONFIGURATIONS) {
        bool x_middle_bit_set = is_middle_bit_set(x);

        for (uint8_t y : VALID_CONFIGURATIONS) {
            if (is_middle_bit_set(y) == x_middle_bit_set) {
                for (uint8_t z : VALID_CONFIGURATIONS) {
                    if (is_middle_bit_set(z) == x_middle_bit_set) {
                        bool cell_lives = does_cell_live(rule, x, y, z);
                        print_mapping(x, y, z, bit_index--, cell_lives);
                    }
                }
            }
        }
    }
}

int main() {

    test();
    // Bitset128 random_bitset = generate_random_bitset128();
    // print_bitset128(random_bitset);

    // uint8_t rule;  // Example rule, analogous to 1D Rule 30
    // std::cin >> rule;
    // generate_3d_mapping(rule);

    return 0;
}