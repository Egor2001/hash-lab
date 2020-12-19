#include "IHashTable.h"
#include "ChainHashTable.h"
#include "OpenLinearAddrHashTable.h"
#include "OpenQuadroAddrHashTable.h"
#include "OpenDoubleAddrHashTable.h"
#include "CuckooHashTable.h"

#include "hashes/IHasher.h"
#include "hashes/HasherAdapter.h"
#include "hashes/TabulationHasher.h"
#include "hashes/PolynomialHasher.h"
#include "hashes/RabinKarpHasher.h"
#include "hashes/AdditionHasher.h"
#include "hashes/Murmur3Hasher.h"
#include "hashes/MdFamilyHasher.h"

#include <iostream>
#include <fstream>

#include <random>
#include <chrono>

#include <algorithm>
#include <vector>
#include <cstring>

using TBenchValue = int;

static constexpr size_t BENCH_SET = 10;

static constexpr size_t BENCH_MIN = 10;
static constexpr size_t BENCH_MAX = 10'000'000;

template<typename TRandGen, typename TKey, 
         template<typename, typename> typename THashTable>
double run_template(TRandGen&& rand_gen, 
                    THashTable<TKey, TBenchValue>* ht, 
                    const std::vector<TKey>& keys, 
                    const std::vector<TKey>& data)
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

template<typename TRandGen, typename TKey>
double run_polymorphic(TRandGen&& rand_gen, 
                       IHashTable<TKey, TBenchValue>* ht, 
                       const std::vector<TKey>& keys, 
                       const std::vector<TKey>& data)
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

template<bool NIsPolymorphic = false, 
         template<typename, typename> typename THashTable>
void bench(std::vector<double>& result_vec)
{
    std::random_device seed_dev;
    std::mt19937 rand_gen(seed_dev());
    std::uniform_int_distribution<size_t> distr;

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

            THashTable<size_t, TBenchValue> ht;
            if constexpr (NIsPolymorphic)
                sum_duration += run_polymorphic(rand_gen, &ht, keys, data);
            else
                sum_duration += run_template(rand_gen, &ht, keys, data);
        }

        result_vec.push_back(sum_duration / (size * BENCH_SET));
    }
}

int main(int argc, char* argv[])
{
    std::vector<double> result;

    if (argc < 2)
    {
        std::cerr << "ERROR: no args\n";
        return 1;
    }

    if (strcmp(argv[1], "chain") == 0)
    {
        bench<false, CChainHashTable>(result);
    }
    else if (strcmp(argv[1], "linear") == 0)
    {
        bench<false, COpenLinearAddrHashTable>(result);
    }
    else if (strcmp(argv[1], "quadro") == 0)
    {
        bench<false, COpenQuadroAddrHashTable>(result);
    }
    else if (strcmp(argv[1], "double") == 0)
    {
        bench<false, COpenDoubleAddrHashTable>(result);
    }
    else
    {
        std::cerr << "ERROR: " << argv[1] << " is not ht\n";
        return 1;
    }

    std::cout << "Size,Time\n";
    size_t count = 0u;
    for (size_t size = BENCH_MIN; size < BENCH_MAX; 
         (size = (size * 3u) / 2u), ++count)
    {
        std::cout << size << ", " << result[count] << "\n";
    }

    return 0;
}
