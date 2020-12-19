#ifndef MD_5_HASHER_H_
#define MD_5_HASHER_H_

#include "IHasher.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

#include <openssl/md5.h>

namespace {

class CMD5Hasher final : public IHasher
{
public:
    CMD5Hasher() = default;

    [[nodiscard]] 
    virtual size_t operator()
        (const uint8_t* data, size_t size) const final override
    {
        MD5(data, size, hash_);

        size_t result = 0u;
        std::memcpy(&result, hash_, sizeof(result));

        return result;
    }

private:
    mutable alignas(sizeof(size_t)) 
    unsigned char hash_[MD5_DIGEST_LENGTH] = {};
};

} // namespace

#endif // MD_5_HASHER_H_
