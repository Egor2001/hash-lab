#ifndef TABULATION_HASHER_H_
#define TABULATION_HASHER_H_

#include "IHasher.h"

#include <cstdint>
#include <cstring>
#include <utility>
#include <stdexcept>

namespace {

class CTabulationHasher final : public IHasher
{
public:
    CTabulationHasher():
        IHasher()
    {
        std::hash<size_t> helper;
        for (size_t tab = 0u; tab < sizeof(size_t); ++tab)
            for (size_t byte = 0u; byte < 0x100; ++byte)
                tabs_[tab][byte] = helper((tab * 0x9e3779b9) ^ byte);
    }

    template<typename TInitFunc>
    explicit CTabulationHasher(TInitFunc func):
        IHasher()
    {
        for (size_t tab = 0u; tab < sizeof(size_t); ++tab)
        {
            for (size_t byte = 0u; byte < 0x100; ++byte)
            {
                tabs_[tab][byte] = func(tab, byte);
            }
        }
    }

    [[nodiscard]]
    virtual size_t operator()
        (const uint8_t* data, size_t size) const noexcept final override
    {
        size_t val = 0u;
        std::memcpy(&val, data, std::min(size, sizeof(val)));

        size_t result = 0u;
        for (size_t tab = 0u; tab < sizeof(size_t); ++tab)
        {
            result ^= tabs_[tab][val & 0xFF];
            val >>= 8u;
        }

        return result;
    }

private:
    size_t tabs_[sizeof(size_t)][256] = {};
};

} // namespace

#endif // TABULATION_HASHER_H_
