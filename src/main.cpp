#include <iostream>
#include <bitset>
#include <array>
#include "random_bitset.hpp"

int main() {
    Bitset128 random_bitset = generate_random_bitset128();

    std::cout << "Random 128-bit Bitset:\n"
              << random_bitset << std::endl;

    generate_3d_mapping(random_bitset);

    return 0;

}