#pragma once
#include <vector>
#include <cstddef>
#include <cstdint>

class BitPackedGrid3D
{
public:
    BitPackedGrid3D(size_t x, size_t y, size_t z);
    BitPackedGrid3D(const BitPackedGrid3D &) = default;
    BitPackedGrid3D &operator=(const BitPackedGrid3D &) = default;
    BitPackedGrid3D(BitPackedGrid3D &&) = default;
    BitPackedGrid3D &operator=(BitPackedGrid3D &&) = default;

    void set(size_t x, size_t y, size_t z, bool value);
    bool get(size_t x, size_t y, size_t z) const;
    void set(size_t idx, bool value);
    bool get(size_t idx) const;

    size_t index(size_t x, size_t y, size_t z) const;
    std::tuple<size_t, size_t, size_t> unpack_bit_index(size_t index) const;
    size_t size_in_bits() const;

    std::vector<uint64_t>::iterator begin();
    std::vector<uint64_t>::iterator end();
    std::vector<uint64_t>::const_iterator begin() const;
    std::vector<uint64_t>::const_iterator end() const;

    bool operator==(const BitPackedGrid3D &other) const;

    size_t x_max, y_max, z_max;

    const std::vector<uint64_t> &raw() const;

private:
    std::vector<uint64_t> data;
};