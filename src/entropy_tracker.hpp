#pragma once
#include <vector>
#include <unordered_map>
#include <cmath>
#include <cstddef>
#include "bit_packed_grid_3d.hpp"

class EntropyTracker
{
public:
    void observe(const BitPackedGrid3D &grid);
    double entropy() const;
    void reset();

private:
    struct VectorHash
    {
        size_t operator()(const std::vector<uint64_t> &vec) const;
    };

    std::unordered_map<std::vector<uint64_t>, size_t, VectorHash> frequency_map;
    size_t total_observations = 0;
};