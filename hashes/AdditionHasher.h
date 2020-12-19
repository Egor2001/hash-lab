#ifndef ADDITION_HASHER_H_
#define ADDITION_HASHER_H_

#include "IHasher.h"

#include <cstdint>
#include <stdexcept>

namespace {

class CAdditionHasher final : public IHasher
{
public:
    explicit CAdditionHasher(size_t mod):
        IHasher(),
        mod_(mod)
    {
        if (mod_ == 0)
            throw std::invalid_argument("error: mod == 0");
    }

    [[nodiscard]]
    virtual size_t operator()
        (const uint8_t* data, size_t size) const noexcept final override
    {
        size_t result = 0u;
        for (size_t index = 0u; index < size; ++index)
            result += data[index];

        result %= mod_;

        return result;
    }

private:
    size_t mod_{ static_cast<size_t>(-1) };
};

} // namespace

#endif // ADDITION_HASHER_H_
