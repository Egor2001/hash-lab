#ifndef SHA_256_HASHER_H_
#define SHA_256_HASHER_H_

#include "IHasher.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

#include <openssl/sha.h>

namespace {

class CSHA256Hasher final : public IHasher
{
public:
    CSHA256Hasher() = default;

    [[nodiscard]] 
    virtual size_t operator()
        (const uint8_t* data, size_t size) const final override
    {
        SHA256(data, size, hash_);

        size_t result = 0u;
        std::memcpy(&result, hash_, sizeof(result));

        return result;
    }

private:
    alignas(sizeof(size_t)) mutable
    unsigned char hash_[SHA256_DIGEST_LENGTH] = {};
};

} // namespace

#endif // SHA_256_HASHER_H_
