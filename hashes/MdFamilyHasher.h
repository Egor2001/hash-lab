#ifndef MD_FAMILY_HASHER_H_
#define MD_FAMILY_HASHER_H_

#include "IHasher.h"

#include <algorithm>
#include <cstdint>

#include <openssl/evp.h>

namespace {

class CMdFamilyHasher : public IHasher
{
public:
    static constexpr const char* DIGEST_SHA256 = "sha256";
    static constexpr const char* DIGEST_MD5 = "md5";

    explicit CMdFamilyHasher(const char* name):
        IHasher(),
        type_(EVP_get_digestbyname(name)),
        context_(EVP_MD_CTX_new())
    {
        if (type_ == nullptr)
            throw std::runtime_error("error: EVP_get_digestbyname()");

        if (context_ == nullptr)
            throw std::runtime_error("error: EVP_MD_CTX_new()");
    }

    CMdFamilyHasher           (const CMdFamilyHasher&) = delete;
    CMdFamilyHasher& operator=(const CMdFamilyHasher&) = delete;

    CMdFamilyHasher(CMdFamilyHasher&& other) noexcept:
        IHasher(std::move(other)),
        type_(other.type_),
        context_(other.context_)
    {
        other.type_ = nullptr;
        other.context_ = nullptr;
    }

    CMdFamilyHasher& operator=(CMdFamilyHasher&& other) noexcept
    {
        this->IHasher::operator=(std::move(other));

        std::swap(type_, other.type_);
        std::swap(context_, other.context_);

        return *this;
    }

    virtual ~CMdFamilyHasher() final
    {
        if (context_ != nullptr)
            EVP_MD_CTX_free(context_);

        context_ = nullptr;
        type_ = nullptr;
    }

    [[nodiscard]] 
    virtual size_t operator()
        (const uint8_t* data, size_t size) const final override
    {
        unsigned int hash_len = 0u;
        std::fill(std::begin(hash_), std::end(hash_), 0x00);

        if (EVP_DigestInit_ex(context_, type_, NULL) != 1)
            throw std::runtime_error("error: EVP_DigestInit_ex()");

        if (EVP_DigestUpdate(context_, data, size) != 1)
            throw std::runtime_error("error: EVP_DigestUpdate()");

        if (EVP_DigestFinal_ex(context_, hash_, &hash_len) != 1)
            throw std::runtime_error("error: EVP_DigestFinal_ex()");

        size_t result = 0u;
        for (unsigned int idx = 0u; idx < hash_len; ++idx)
            result ^= (static_cast<size_t>(hash_[idx]) + 0x9e3779b9 + 
                       (result << 6) + (result >> 2));

        return result;
    }

private:
    const EVP_MD* type_ = NULL;
    EVP_MD_CTX* context_ = NULL;

    mutable unsigned char hash_[EVP_MAX_MD_SIZE] = {};
};

} // namespace

#endif // MD_FAMILY_HASHER_H_
