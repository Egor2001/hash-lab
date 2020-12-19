#include "IHasher.h"
#include "HasherAdapter.h"
#include "TabulationHasher.h"
#include "PolynomialHasher.h"
#include "RabinKarpHasher.h"
#include "AdditionHasher.h"
#include "Murmur3Hasher.h"
#include "MdFamilyHasher.h"

#include <utility>
#include <iostream>

int main()
{
    auto tabs_init = [helper = std::hash<size_t>()]
        ([[maybe_unused]] size_t tab, [[maybe_unused]] size_t byte) -> size_t
    {
        return helper((tab * 0x9e3779b9) ^ byte);
    };

    const size_t mod = 1000000007;

    CHasherAdapter<CTabulationHasher> 
        tabulation_hasher{ CTabulationHasher(tabs_init) };
    CHasherAdapter<CPolynomialHasher> 
        polynomial_hasher{ CPolynomialHasher(mod, { 3, 5, 7, 11 }) };
    CHasherAdapter<CRabinKarpHasher> 
        rabin_karp_hasher{ CRabinKarpHasher(mod, 7) };
    CHasherAdapter<CAdditionHasher> 
        addition_hasher{ CAdditionHasher(mod) };

    CHasherAdapter<CMurmur3Hasher> 
        murmur3_hasher{ CMurmur3Hasher(0xb1bab0ba) };
    CHasherAdapter<CMdFamilyHasher> 
        sha256_hasher{ CMdFamilyHasher("sha256") };
    CHasherAdapter<CMdFamilyHasher> 
        md5_hasher{ CMdFamilyHasher("md5") };

    std::cerr << "TABULATION(0xb1bab0ba) = " << 
        tabulation_hasher(0xb1bab0ba) << '\n';
    std::cerr << "POLYNOMIAL(0xb1bab0ba) = " << 
        polynomial_hasher(0xb1bab0ba) << '\n';
    std::cerr << "RABIN_KARP(\"b1bab0ba\") = " << 
        rabin_karp_hasher("b1bab0ba") << '\n';
    std::cerr << "ADDITION(\"b1bab0ba\") = " << 
        addition_hasher("b1bab0ba") << '\n';

    std::cerr << "MURMUR3(\"b1bab0ba\") = " << 
        murmur3_hasher("b1bab0ba") << '\n';
    std::cerr << "SHA256(\"b1bab0ba\") = " << 
        sha256_hasher("b1bab0ba") << '\n';
    std::cerr << "MD5(\"b1bab0ba\") = " << 
        md5_hasher("b1bab0ba") << '\n';

    return 0;
}
