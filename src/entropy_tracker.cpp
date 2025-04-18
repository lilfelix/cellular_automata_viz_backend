#include "entropy_tracker.hpp"

void EntropyTracker::observe(const BitPackedGrid3D &grid)
{
    const auto &data = grid.raw(); // Returns const std::vector<uint64_t>&
    ++frequency_map[data];
    ++total_observations;
}

double EntropyTracker::entropy() const
{
    double H = 0.0;
    for (const auto &kv : frequency_map)
    {
        double p = static_cast<double>(kv.second) / total_observations;
        H -= p * std::log2(p);
    }
    return H;
}

void EntropyTracker::reset()
{
    frequency_map.clear();
    total_observations = 0;
}

size_t EntropyTracker::VectorHash::operator()(const std::vector<uint64_t> &vec) const
{
    size_t hash = 0xcbf29ce484222325;
    for (uint64_t v : vec)
    {
        hash ^= v;
        hash *= 0x100000001b3; // FNV-1a
    }
    return hash;
}