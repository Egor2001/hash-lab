#ifndef POLYNOMIAL_HASHER_H_
#define POLYNOMIAL_HASHER_H_

#include "IHasher.h"

#include <cstdint>
#include <stdexcept>

namespace {

class CPolynomialHasher final : public IHasher
{
public:
    static constexpr size_t BOUND_DEGREE = 8u;

    explicit CPolynomialHasher(size_t mod, std::initializer_list<size_t> poly):
        IHasher(),
        mod_(mod),
        deg_(poly.size())
    {
        if (mod_ == 0)
            throw std::invalid_argument("error: mod == 0");

        if (deg_ >= BOUND_DEGREE)
            throw std::invalid_argument("error: deg_ >= BOUND_DEGREE");

        size_t index = 0u;
        for (auto it = std::rbegin(poly); it != std::rend(poly); ++it)
        {
            poly_[index] = *it;

            ++index;
            if (index >= BOUND_DEGREE)
                break;
        }
    }

    // Is needed just for simplicity
    CPolynomialHasher():
        CPolynomialHasher(1000000007, { 3, 5, 7, 11 })
    {}

    [[nodiscard]]
    virtual size_t operator()
        (const uint8_t* data, size_t size) const noexcept final override
    {
        size_t val = 0u;
        std::memcpy(&val, data, std::min(size, sizeof(val)));

        size_t result = 0u;
        for (size_t index = 0u; index < deg_; ++index)
            result = (result * val + poly_[index]) % mod_;

        return result;
    }

private:
    size_t mod_{ static_cast<size_t>(-1) };
    size_t deg_{ 1u };
    size_t poly_[BOUND_DEGREE] = { 1u, };
};

} // namespace

#endif // POLYNOMIAL_HASHER_H_
