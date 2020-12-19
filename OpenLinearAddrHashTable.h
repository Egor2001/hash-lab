#ifndef OPEN_LINEAR_ADDR_HASHTABLE_H_
#define OPEN_LINEAR_ADDR_HASHTABLE_H_

#include "IOpenAddrHashTable.h"

#include <utility>
#include <optional>
#include <functional>
#include <stdexcept>

namespace {

template<class TK, class TV, class TH = std::hash<TK>, size_t NLR = 4u>
class COpenLinearAddrHashTable final : public IOpenAddrHashTable<TK, TV>
{
public:
    using typename IOpenAddrHashTable<TK, TV>::TKey;
    using typename IOpenAddrHashTable<TK, TV>::TValue;
    using typename IOpenAddrHashTable<TK, TV>::TData;
                            
    using IOpenAddrHashTable<TK, TV>::NStartCapacity;
    using IOpenAddrHashTable<TK, TV>::NRehashFactor;

    using THasher = TH;
    static constexpr size_t NLoadRatio = NLR;

    COpenLinearAddrHashTable() = default;

    template<typename TIter>
    COpenLinearAddrHashTable(TIter begin_it, TIter end_it):
        COpenLinearAddrHashTable()
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
        return (hash_vec_[index] + count) % this->capacity();
    }

    [[nodiscard]]
    virtual size_t pos(const TKey& desired) const noexcept override final
    {
        size_t hash = hasher_(desired);
        size_t index = hash % this->capacity();
        hash_vec_[index] = hash;

        return index;
    }

    [[nodiscard]]
    virtual size_t get_load_ratio() const noexcept override final
    {
        return NLoadRatio;
    }

    virtual void rehash(size_t new_capacity) override final
    {
        hash_vec_.resize(new_capacity, 0u);
        this->IOpenAddrHashTable<TK, TV>::rehash(new_capacity);
    }

private:
    THasher hasher_{};
    mutable std::vector<size_t> hash_vec_ = 
        std::vector<size_t>(NStartCapacity, 0u);
};

} // namespace

#endif // OPEN_LINEAR_ADDR_HASHTABLE_H_
