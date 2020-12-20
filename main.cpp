#include "IHashTable.h"
#include "IOpenAddrHashTable.h"
#include "OpenLinearAddrHashTable.h"
#include "OpenQuadroAddrHashTable.h"
#include "OpenDoubleAddrHashTable.h"
#include "ChainHashTable.h"
#include "CuckooHashTable.h"

#include "IHasher.h"
#include "HasherAdapter.h"
#include "TabulationHasher.h"
#include "PolynomialHasher.h"
#include "RabinKarpHasher.h"
#include "AdditionHasher.h"
#include "Murmur3Hasher.h"
#include "SHA256Hasher.h"
#include "MD5Hasher.h"
#include "MdFamilyHasher.h"

#include <iostream>
#include <fstream>

#include <random>
#include <chrono>

#include <algorithm>
#include <vector>
#include <cstring>

using TBenchKey = size_t;// std::string;
using TBenchValue = int;

static constexpr bool BENCH_IS_POLYMORPHIC = false;

static constexpr size_t BENCH_SET = 10;

static constexpr size_t BENCH_MIN = 10;
static constexpr size_t BENCH_MAX = 100'000'000;

template<typename THash> // "linear"
using TLinearHT = COpenLinearAddrHashTable<TBenchKey, TBenchValue, THash>;

template<typename THash> // "quadro"
using TQuadroHT = COpenQuadroAddrHashTable<TBenchKey, TBenchValue, THash>;

template<typename THash> // "double"
using TDoubleHT = COpenDoubleAddrHashTable<TBenchKey, TBenchValue, THash, 
                                           std::hash<TBenchKey>>;

template<typename THash> // "chain75"
using TChain75HT = CChainHashTable<TBenchKey, TBenchValue, THash, 4u>;

template<typename THash> // "chain95"
using TChain95HT = CChainHashTable<TBenchKey, TBenchValue, THash, 20u>;

template<typename THash> // "cuckoo"
using TCuckooHT = CCuckooHashTable<TBenchKey, TBenchValue, THash, 
                                   std::hash<TBenchKey>>;

// "std"
using TStdHF = std::hash<TBenchKey>;
// "murmur3"
using TMurmurHF = CHasherAdapter<CMurmur3Hasher>;
// "sha256"
using TShaHF = CHasherAdapter<CSHA256Hasher>;
// "md5"
using TMdHF = CHasherAdapter<CMD5Hasher>;
// "polynomial"
using TPolynomialHF = CHasherAdapter<CPolynomialHasher>;
// "tabulation"
using TTabulationHF = CHasherAdapter<CTabulationHasher>;
// "rabinkarp"
using TRabinKarpHF = CHasherAdapter<CRabinKarpHasher>;
// "addition"
using TAdditionHF = CHasherAdapter<CAdditionHasher>;

template<typename TRandGen, typename THashTable>
double run_template(TRandGen&& rand_gen, THashTable* ht, 
                    const std::vector<TBenchKey>& keys, 
                    const std::vector<TBenchKey>& data);

template<typename TRandGen>
double run_polymorphic(TRandGen&& rand_gen, 
                       IHashTable<TBenchKey, TBenchValue>* ht, 
                       const std::vector<TBenchKey>& keys, 
                       const std::vector<TBenchKey>& data);

template<typename THashTable>
std::vector<double> bench();

std::vector<double>
launch_table(std::string_view table_name, 
             std::string_view hash_name);

template<template<typename> typename TTableType>
std::vector<double>
launch_hash(std::string_view hash_name);

template<template<typename> typename TTableType, typename THashType>
std::vector<double>
launch_bench();

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        std::cerr << "USAGE: " << argv[0] << 
            " TABLE_TYPE HASHER_TYPE OUTFILE\n";
        return 1;
    }

    std::vector<double> result;
    try {
        result = launch_table(argv[1], argv[2]);
    }
    catch (std::exception& exc)
    {
        std::cerr << exc.what() << '\n';
        std::cerr << 
            "TABLE TYPES:\n" 
            "linear quadro double chain75 chain95 cuckoo\n";
        std::cerr << 
            "HASHER TYPES:\n" 
            "std murmur3 sha256 md5 polynomial tabulation rabinkarp addition\n";
        return 1;
    }

    std::ofstream fout;
    try {
        fout = std::ofstream(argv[3]);
    }
    catch (std::exception& exc)
    {
        std::cerr << exc.what() << '\n';
        return 1;
    }

    fout << "Size,Time\n";
    size_t count = 0u;
    for (size_t size = BENCH_MIN; size < BENCH_MAX; 
         (size = (size * 3u) / 2u), ++count)
    {
        fout << size << ", " << result[count] << "\n";
    }

    return 0;
}

template<typename TRandGen, typename THashTable>
double run_template(TRandGen&& rand_gen, THashTable* ht, 
                    const std::vector<TBenchKey>& keys, 
                    const std::vector<TBenchKey>& data)
{
    auto distr = std::uniform_int_distribution<>(0, 1);
    auto start = std::chrono::high_resolution_clock::now();
    for (auto& key : keys)
        ht->insert(key, TBenchValue{});

    for (auto& key : data)
    {
        distr(std::forward<TRandGen>(rand_gen)) == 0 ? 
            static_cast<void>(ht->erase(key)) : 
            static_cast<void>(ht->find(key)); 
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::
        duration_cast<std::chrono::nanoseconds>(end - start).count();
}

template<typename TRandGen>
double run_polymorphic(TRandGen&& rand_gen, 
                       IHashTable<TBenchKey, TBenchValue>* ht, 
                       const std::vector<TBenchKey>& keys, 
                       const std::vector<TBenchKey>& data)
{
    auto distr = std::uniform_int_distribution<>(0, 1);
    auto start = std::chrono::high_resolution_clock::now();
    for (auto& key : keys)
        ht->insert(key, TBenchValue{});

    for (auto& key : data)
    {
        distr(std::forward<TRandGen>(rand_gen)) == 0 ? 
            static_cast<void>(ht->erase(key)) : 
            static_cast<void>(ht->find(key)); 
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::
        duration_cast<std::chrono::nanoseconds>(end - start).count();
}

template<typename THashTable>
std::vector<double> bench()
{
    std::random_device seed_dev;
    std::mt19937 rand_gen(seed_dev());
    std::uniform_int_distribution<size_t> distr;

    std::vector<double> result;
    std::vector<size_t> keys, data;
    size_t count = 0u;
    for (size_t size = BENCH_MIN; size < BENCH_MAX; 
         (size = (size * 3u) / 2u), ++count)
    {
        keys.resize(size);
        data.resize(size);

        double sum_duration = 0.0;
        for (size_t set_idx = 0u; set_idx < BENCH_SET; ++set_idx)
        {
            for (size_t idx = 0u; idx < size; ++idx)
            {
                keys[idx] = distr(rand_gen);
                data[idx] = ((idx * 2u) < size ? keys[idx] : distr(rand_gen));
            }
            std::shuffle(std::begin(data), std::end(data), rand_gen);

            THashTable ht;
            if constexpr (BENCH_IS_POLYMORPHIC)
                sum_duration += run_polymorphic(rand_gen, &ht, keys, data);
            else
                sum_duration += run_template(rand_gen, &ht, keys, data);
        }

        result.push_back(sum_duration / (size * BENCH_SET));
    }

    return result;
}

std::vector<double>
launch_table(std::string_view table_name, 
             std::string_view hash_name)
{
    if (table_name == "linear")
        return launch_hash<TLinearHT>(hash_name);
    if (table_name == "quadro")
        return launch_hash<TQuadroHT>(hash_name);
    if (table_name == "double")
        return launch_hash<TDoubleHT>(hash_name);
    if (table_name == "chain75")
        return launch_hash<TChain75HT>(hash_name);
    if (table_name == "chain95")
        return launch_hash<TChain95HT>(hash_name);
    if (table_name == "cuckoo")
        return launch_hash<TCuckooHT>(hash_name);

    throw std::invalid_argument("error: no such table type");
}

template<template<typename> typename TTableType>
std::vector<double>
launch_hash(std::string_view hash_name)
{
    if (hash_name == "std")
        return launch_bench<TTableType, TStdHF>();
    if (hash_name == "murmur3")
        return launch_bench<TTableType, TMurmurHF>();
    if (hash_name == "sha256")
        return launch_bench<TTableType, TShaHF>();
    if (hash_name == "md5")
        return launch_bench<TTableType, TMdHF>();
    if (hash_name == "polynomial")
        return launch_bench<TTableType, TPolynomialHF>();
    if (hash_name == "tabulation")
        return launch_bench<TTableType, TTabulationHF>();
    if (hash_name == "rabinkarp")
        return launch_bench<TTableType, TRabinKarpHF>();
    if (hash_name == "addition")
        return launch_bench<TTableType, TAdditionHF>();
    
    throw std::invalid_argument("error: no such hash type");
}

template<template<typename> typename TTableType, typename THashType>
std::vector<double>
launch_bench()
{
    return bench<TTableType<THashType>>();
}
