#include "bit_packed_grid_3d.hpp"

const std::vector<uint64_t> &BitPackedGrid3D::raw() const { return data; }

BitPackedGrid3D::BitPackedGrid3D(size_t x, size_t y, size_t z)
    : x_max(x), y_max(y), z_max(z),
      data(((x * y * z) + 63) / 64, 0) {}

void BitPackedGrid3D::set(size_t x, size_t y, size_t z, bool value)
{
    size_t idx = index(x, y, z);
    size_t word = idx / 64;
    size_t offset = idx % 64;
    if (value)
        data[word] |= (1ULL << offset);
    else
        data[word] &= ~(1ULL << offset);
}

bool BitPackedGrid3D::get(size_t x, size_t y, size_t z) const
{
    size_t idx = index(x, y, z);
    size_t word = idx / 64;
    size_t offset = idx % 64;
    return (data[word] >> offset) & 1ULL;
}

void BitPackedGrid3D::set(size_t idx, bool value)
{
    size_t word = idx / 64;
    size_t offset = idx % 64;
    if (value)
        data[word] |= (1ULL << offset);
    else
        data[word] &= ~(1ULL << offset);
}

bool BitPackedGrid3D::get(size_t idx) const
{
    size_t word = idx / 64;
    size_t offset = idx % 64;
    return (data[word] >> offset) & 1ULL;
}

size_t BitPackedGrid3D::index(size_t x, size_t y, size_t z) const
{
    return x * y_max * z_max + y * z_max + z;
}

// Usage:
// auto [x, y, z] = unpack_bit_index(index, y_max, z_max);
std::tuple<size_t, size_t, size_t> BitPackedGrid3D::unpack_bit_index(size_t index) const
{
    size_t x = index / (y_max * z_max);
    size_t rem = index % (y_max * z_max);
    size_t y = rem / z_max;
    size_t z = rem % z_max;
    return {x, y, z};
}

size_t BitPackedGrid3D::size_in_bits() const
{
    return x_max * y_max * z_max;
}

std::vector<uint64_t>::iterator BitPackedGrid3D::begin()
{
    return data.begin();
}

std::vector<uint64_t>::iterator BitPackedGrid3D::end()
{
    return data.end();
}

std::vector<uint64_t>::const_iterator BitPackedGrid3D::begin() const
{
    return data.begin();
}

std::vector<uint64_t>::const_iterator BitPackedGrid3D::end() const
{
    return data.end();
}

bool BitPackedGrid3D::operator==(const BitPackedGrid3D &other) const
{
    if (x_max != other.x_max || y_max != other.y_max || z_max != other.z_max)
        return false;

    return std::equal(data.begin(), data.end(), other.data.begin());
}