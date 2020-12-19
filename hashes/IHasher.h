#ifndef IHASHER_H_
#define IHASHER_H_

#include <cstdint>
#include <cstddef>

namespace {

class IHasher
{
public:
    IHasher() = default;
    virtual ~IHasher() = default;

    [[nodiscard]]
    virtual size_t operator()(const uint8_t* data, size_t size) const = 0;
};

} // namespace

#endif // IHASHER_H_
