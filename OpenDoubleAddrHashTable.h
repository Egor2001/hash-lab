#ifndef OPEN_DOUBLE_ADDR_HASHTABLE_H_
#define OPEN_DOUBLE_ADDR_HASHTABLE_H_

#include "IOpenAddrHashTable.h"

#include <utility>
#include <optional>
#include <functional>
#include <stdexcept>

namespace {

template<class TK, class TV, 
         class TBH = std::hash<TK>, class TIH = std::hash<TK>, size_t NLR = 4u>
class COpenDoubleAddrHashTable final : public IOpenAddrHashTable<TK, TV>
{
public:
    using typename IOpenAddrHashTable<TK, TV>::TKey;
    using typename IOpenAddrHashTable<TK, TV>::TValue;
    using typename IOpenAddrHashTable<TK, TV>::TData;
                            
    using IOpenAddrHashTable<TK, TV>::NStartCapacity;
    using IOpenAddrHashTable<TK, TV>::NRehashFactor;

    using TBaseHasher = TBH;
    using TIterHasher = TIH;
    static constexpr size_t NLoadRatio = NLR;

    COpenDoubleAddrHashTable() = default;

    template<typename TIter>
    COpenDoubleAddrHashTable(TIter begin_it, TIter end_it):
        COpenDoubleAddrHashTable()
    {
        for (auto it = begin_it; it != end_it; ++it)
        {
            auto& [key, value] = *it;
            insert(key, value);
        }
    }

    virtual bool insert(const TKey& key, const TValue& value) override final
    {
        return this->IOpenAddrHashTable<TK, TV>::insert(key, value);
    }

    virtual bool erase(const TKey& desired) override final
    {
        return this->IOpenAddrHashTable<TK, TV>::erase(desired);
    }

    [[nodiscard]]
    virtual std::optional<std::reference_wrapper<TValue>> 
        find(const TKey& desired) override final
    {
        return this->IOpenAddrHashTable<TK, TV>::find(desired);
    }

    [[nodiscard]]
    virtual std::optional<std::reference_wrapper<const TValue>> 
        find(const TKey& desired) const override final
    {
        return this->IOpenAddrHashTable<TK, TV>::find(desired);
    }

protected:
    [[nodiscard]]
    virtual size_t run(size_t index, size_t count) const noexcept override final
    {
        return (base_hash_vec_[index] + 
                count * iter_hash_vec_[index]) % this->capacity();
    }

    [[nodiscard]]
    virtual size_t pos(const TKey& desired) const noexcept override final
    {
        size_t base_hash = base_hasher_(desired);
        // This transformation is used in order to make hash odd
        size_t iter_hash = 2 * iter_hasher_(desired) + 1;

        size_t index = base_hash % this->capacity();
        base_hash_vec_[index] = base_hash;
        iter_hash_vec_[index] = iter_hash;

        return index;
    }

    [[nodiscard]]
    virtual size_t get_load_ratio() const noexcept override final
    {
        return NLoadRatio;
    }

    virtual void rehash(size_t new_capacity) override final
    {
        base_hash_vec_.resize(new_capacity, 0u);
        iter_hash_vec_.resize(new_capacity, 0u);
        this->IOpenAddrHashTable<TK, TV>::rehash(new_capacity);
    }

private:
    TBaseHasher base_hasher_{};
    TIterHasher iter_hasher_{};
    mutable std::vector<size_t> base_hash_vec_ = 
        std::vector<size_t>(NStartCapacity, 0u);
    mutable std::vector<size_t> iter_hash_vec_ = 
        std::vector<size_t>(NStartCapacity, 0u);
};

} // namespace

#endif // OPEN_DOUBLE_ADDR_HASHTABLE_H_
