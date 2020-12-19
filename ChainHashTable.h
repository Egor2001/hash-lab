#ifndef CHAIN_HASHTABLE_H_
#define CHAIN_HASHTABLE_H_

#include "IHashTable.h"

#include <utility>
#include <optional>
#include <functional>
#include <stdexcept>

namespace {

template<class TK, class TV, class TH = std::hash<TK>, size_t NLR = 4>
class CChainHashTable final : public IHashTable<TK, TV>
{
public:
    static constexpr size_t NLoadRatio = NLR;

    using typename IHashTable<TK, TV>::TKey;
    using typename IHashTable<TK, TV>::TValue;
    using typename IHashTable<TK, TV>::TData;
    using THasher = TH;

    static constexpr size_t NStartCapacity = NLoadRatio;
    static constexpr double NRehashFactor = 2.0;

    CChainHashTable() = default;

    template<typename TIter>
    CChainHashTable(TIter begin_it, TIter end_it):
        CChainHashTable()
    {
        for (TIter iter = begin_it; iter != end_it; ++iter)
        {
            const auto& [key, value] = *iter;
            insert(key, value); // Safe as class is `final`
        }
    }

    [[nodiscard]]
    virtual size_t size() const noexcept override final
    {
        return size_;
    }

    [[nodiscard]]
    virtual size_t capacity() const noexcept override final
    {
        return chain_vec_.size();
    }

    [[nodiscard]]
    virtual bool empty() const noexcept override final
    {
        return size_ == 0u;
    }

    virtual bool insert(const TKey& desired, 
                        const TValue& desired_value) override final
    {
        if (chain_vec_.size() * (NLoadRatio - 1) < (size_ + 1) * NLoadRatio)
            rehash(chain_vec_.size() * NRehashFactor);

        size_t index = hasher_(desired) % chain_vec_.size();
        if (auto it = search(desired, index); 
            it == std::end(chain_vec_[index]))
        {
            chain_vec_[index].emplace_front(desired, desired_value);
            ++size_;
            return true;
        }
        else
        {
            auto& [key, value] = *it;
            value = desired_value;
        }

        return false;
    }

    virtual bool erase(const TKey& desired) override final
    {
        size_t index = hasher_(desired) % chain_vec_.size();
        if (auto it = search(desired, index); 
            it != std::end(chain_vec_[index]))
        {
            chain_vec_[index].erase(it);
            --size_;
            return true;
        }

        return false;
    }

    [[nodiscard]]
    virtual std::optional<std::reference_wrapper<TValue>> 
        find(const TKey& desired) override final
    {
        auto result = const_cast<const CChainHashTable&>(*this).find(desired);

        return (result ? 
                std::make_optional(
                        std::ref(const_cast<TValue&>(result.value().get()))
                    ) : 
                std::nullopt);
    }

    [[nodiscard]]
    virtual std::optional<std::reference_wrapper<const TValue>> 
        find(const TKey& desired) const override final
    {
        size_t index = hasher_(desired) % chain_vec_.size();
        if (auto it = search(desired, index); 
            it != std::end(chain_vec_[index]))
        {
            auto& [key, value] = *it;
            return std::make_optional(std::cref(value));
        }

        return std::nullopt;
    }

protected:
    void rehash(size_t new_capacity)
    {
        if (new_capacity <= chain_vec_.size())
            throw std::invalid_argument(
                    "CChainHashTable::rehash(): "
                    "new_capacity <= chain_vec_.size()"
                    );

        auto old_chain_vec = std::vector<std::list<TData>>(new_capacity);
        std::swap(chain_vec_, old_chain_vec);

        size_ = 0u;
        for (auto& chain : old_chain_vec)
        {
            for (auto& [key, value] : chain)
                insert(std::move(key), std::move(value));
        }
    }

    auto search(const TKey& desired, size_t index) const
    {
        return const_cast<CChainHashTable&>(*this).search(desired, index);
    }

    auto search(const TKey& desired, size_t index)
    {
        return std::find_if(
                std::begin(chain_vec_[index]), 
                std::end(chain_vec_[index]), 
                [&desired](const auto& elem) { 
                    auto& [key, value] = elem;
                    return key == desired; 
                }
            );
    }

private:
    size_t size_{};
    THasher hasher_{};

    // TODO: to replace std::list with custom array-based implementation
    std::vector<std::list<TData>> chain_vec_{ NStartCapacity };
};

} // namespace

#endif // CHAIN_HASHTABLE_H_
