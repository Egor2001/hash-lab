#ifndef MURMUR_3_HASHER_H_
#define MURMUR_3_HASHER_H_

#include "IHasher.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

namespace {

class CMurmur3Hasher final : public IHasher
{
public:
    explicit CMurmur3Hasher(size_t seed):
        IHasher(),
        seed_(seed)
    {}

    [[nodiscard]] 
    virtual size_t operator()
        (const uint8_t* data, size_t size) const noexcept final override
    {
        return calculate(data, size);
    }

protected:
    [[nodiscard]]
    static inline uint32_t scramble(uint32_t val) noexcept
    {
        val ^= val >> 16;
        val *= 0x85ebca6bUL;
        val ^= val >> 13;
        val *= 0xc2b2ae35UL;
        val ^= val >> 16;

        return val;
    }

    [[nodiscard]]
    uint32_t calculate(const uint8_t* data, size_t size) const noexcept
    {
        uint32_t hash = seed_, temp = 0u;
        for (size_t idx = size / sizeof(uint32_t); idx > 0u; --idx)
        {
            std::memcpy(&temp, data, sizeof(uint32_t));
            data += sizeof(uint32_t);

            hash ^= scramble(temp);
            hash = (hash << 13) | (hash >> 19);
            hash = hash * 5 + 0xe6546b64UL;
        }

        temp = 0u;
        switch (size % sizeof(uint32_t))
        {
            case 3: temp |= data[2] << 16; [[fallthrough]];
            case 2: temp |= data[1] << 8; [[fallthrough]];
            case 1: temp |= data[0]; [[fallthrough]];
            default: break;
        }

        hash ^= scramble(temp);
        hash ^= size;

        hash ^= hash >> 16;
        hash *= 0x85ebca6bUL;
        hash ^= hash >> 13;
        hash *= 0xc2b2ae35UL;
        hash ^= hash >> 16;

        return hash;
    }

private:
    size_t seed_{};
};

} // namespace

#endif // MURMUR_3_HASHER_H_
