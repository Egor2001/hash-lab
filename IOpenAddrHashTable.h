#ifndef OPEN_ADDR_HASHTABLE_H_
#define OPEN_ADDR_HASHTABLE_H_

#include "IHashTable.h"

#include <iostream>

#include <new>
#include <utility>
#include <optional>
#include <functional>
#include <stdexcept>

namespace {

template<class TK, class TV>
class IOpenAddrHashTable : public IHashTable<TK, TV>
{
public:
    using typename IHashTable<TK, TV>::TKey;
    using typename IHashTable<TK, TV>::TValue;
    using typename IHashTable<TK, TV>::TData;

    using TStorage = 
        typename std::aligned_storage<sizeof(TData), alignof(TData)>::type;

    static constexpr size_t NStartCapacity = 1u;
    static constexpr double NRehashFactor = 2.0;

    IOpenAddrHashTable() = default;

    [[nodiscard]]
    virtual size_t size() const noexcept override final
    {
        return size_;
    }

    [[nodiscard]]
    virtual size_t capacity() const noexcept override final
    {
        return data_vec_.size();
    }

    [[nodiscard]]
    virtual bool empty() const noexcept override final
    {
        return size_ == 0u;
    }

    virtual bool insert(const TKey& key, const TValue& value) override = 0;

    virtual bool erase(const TKey& desired) override = 0;

    [[nodiscard]]
    virtual std::optional<std::reference_wrapper<TValue>> 
        find(const TKey& key) override = 0;

    [[nodiscard]]
    virtual std::optional<std::reference_wrapper<const TValue>> 
        find(const TKey& desired) const override = 0;

protected:
    template<typename... Types>
    inline TData* construct_at(size_t idx, Types&&... args)
    {
        return new (&data_vec_[idx]) TData{ std::forward<Types>(args)... };
    }

    inline void destruct_at(size_t idx)
    {
        std::launder(reinterpret_cast<TData*>(&data_vec_[idx]))->~TData();
    }

    [[nodiscard]]
    inline const TData& get_data_at(size_t idx) const noexcept
    {
        return const_cast<IOpenAddrHashTable*>(this)->get_data_at(idx);
    }

    [[nodiscard]]
    inline TData& get_data_at(size_t idx) noexcept
    {
        return *std::launder(reinterpret_cast<TData*>(&data_vec_[idx]));
    }

    [[nodiscard]]
    virtual size_t run(size_t index, size_t count) const noexcept = 0;

    [[nodiscard]]
    virtual size_t pos(const TKey& desired) const noexcept = 0;

    [[nodiscard]]
    virtual size_t get_load_ratio() const noexcept = 0;

    virtual void rehash(size_t new_capacity) = 0;

private:
    size_t size_{};

    std::vector<bool> skip_vec_ = std::vector<bool>(NStartCapacity, false);
    std::vector<bool> used_vec_ = std::vector<bool>(NStartCapacity, false);

    std::vector<TStorage> data_vec_ = std::vector<TStorage>(NStartCapacity);
};

template<class TK, class TV>
bool IOpenAddrHashTable<TK, TV>::
insert(const TKey& desired, const TValue& desired_value)
{
    size_t ratio = get_load_ratio();
    if (data_vec_.size() * (ratio - 1) < (size_ + 1) * ratio)
        rehash(data_vec_.size() * NRehashFactor);

    size_t target = data_vec_.size();
    size_t index = pos(desired), offset = index;
    for (size_t count = 0u;
         (used_vec_[offset] || skip_vec_[offset]) && (count < data_vec_.size());
         ++count, offset = run(index, count))
    {
        if (skip_vec_[offset] && (target == data_vec_.size()))
            target = offset;

        if (used_vec_[offset])
        {
            if (auto& [key, value] = get_data_at(offset); key == desired)
            {
                value = desired_value;
                return false;
            }
        }
    }

    if (target == data_vec_.size())
        target = offset;

    construct_at(target, desired, desired_value);
    used_vec_[target] = true;
    skip_vec_[target] = false;
    ++size_;

    return true;
}

template<class TK, class TV>
bool IOpenAddrHashTable<TK, TV>::
erase(const TKey& desired)
{
    for (size_t index = pos(desired), offset = index, count = 0u;
         (used_vec_[offset] || skip_vec_[offset]) && (count < data_vec_.size());
         ++count, offset = run(index, count))
    {
        if (auto& [key, value] = get_data_at(offset); 
            used_vec_[offset] && key == desired)
        {
            destruct_at(offset);
            used_vec_[offset] = false;
            skip_vec_[offset] = true;
            --size_;

            return true;
        }
    }

    return false;
}


template<class TK, class TV>
std::optional<
    std::reference_wrapper<
        const typename IOpenAddrHashTable<TK, TV>::TValue
        >
    > 
IOpenAddrHashTable<TK, TV>::
find(const TKey& key) const
{
    auto result = const_cast<IOpenAddrHashTable&>(*this).find(key);

    return (result ? 
            std::make_optional(
                    std::cref(const_cast<TValue&>(result.value().get()))
                ) : 
            std::nullopt);
}

template<class TK, class TV>
std::optional<
    std::reference_wrapper<
        typename IOpenAddrHashTable<TK, TV>::TValue
        >
    > 
IOpenAddrHashTable<TK, TV>::
find(const TKey& desired)
{
    for (size_t index = pos(desired), offset = index, count = 0u;
         (used_vec_[offset] || skip_vec_[offset]) && (count < data_vec_.size());
         ++count, offset = run(index, count))
    {
        if (auto& [key, value] = get_data_at(offset); 
            used_vec_[offset] && key == desired)
            return std::ref(value);
    }

    return std::nullopt;
}

template<class TK, class TV>
void IOpenAddrHashTable<TK, TV>::
rehash(size_t new_capacity)
{
    if (new_capacity <= data_vec_.size())
        throw std::invalid_argument(
                "IOpenAddrHashTable::rehash(): "
                "new_capacity <= data_vec_.size()"
                );

    auto old_used_vec = std::vector<bool>(new_capacity);
    auto old_data_vec = std::vector<TStorage>(new_capacity);

    skip_vec_ = std::vector<bool>(new_capacity, false);
    std::swap(used_vec_, old_used_vec);
    std::swap(data_vec_, old_data_vec);

    size_ = 0u;
    for (size_t index = 0u; index < old_data_vec.size(); ++index)
    {
        if (old_used_vec[index])
        {
            TData* ptr = 
                std::launder(reinterpret_cast<TData*>(&old_data_vec[index]));

            auto&& [key, value] = std::move(*ptr);
            insert(std::move(key), std::move(value));

            ptr->~TData();
            old_used_vec[index] = false;
        }
    }
}

} // namespace

#endif // OPEN_ADDR_HASHTABLE_H_
