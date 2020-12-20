#ifndef RABIN_KARP_HASHER_H_
#define RABIN_KARP_HASHER_H_

#include "IHasher.h"

#include <cstdint>
#include <stdexcept>

namespace {

class CRabinKarpHasher final : public IHasher
{
public:
    explicit CRabinKarpHasher(size_t mod, size_t val):
        IHasher(),
        mod_(mod),
        val_(val)
    {
        if (mod_ == 0)
            throw std::invalid_argument("error: mod == 0");

        if (val_ % mod_ == 0)
            throw std::invalid_argument("error: val_ % mod_ == 0");
    }

    // Is needed just for simplicity
    CRabinKarpHasher():
        CRabinKarpHasher(1000000007, 17)
    {}

    [[nodiscard]]
    virtual size_t operator()
        (const uint8_t* data, size_t size) const noexcept final override
    {
        size_t result = 0u;
        for (size_t index = 0u; index < size; ++index)
            result = (result * val_ + data[index]) % mod_;

        return result;
    }

private:
    size_t mod_{ static_cast<size_t>(-1) };
    size_t val_{ 1u };
};

} // namespace

#endif // RABIN_KARP_HASHER_H_
