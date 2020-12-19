#ifndef CUCKOO_HASHTABLE_H_
#define CUCKOO_HASHTABLE_H_

#include "IHashTable.h"

#include <iostream>

#include <utility>
#include <optional>
#include <functional>
#include <stdexcept>
#include <random>

namespace {

template<class TK, class TV, 
         class TLH = std::hash<TK>, class TRH = std::hash<TK>>
class CCuckooHashTable final : public IHashTable<TK, TV>
{
public:
    using typename IHashTable<TK, TV>::TKey;
    using typename IHashTable<TK, TV>::TValue;
    using typename IHashTable<TK, TV>::TData;
    using TLeftHasher = TLH;
    using TRightHasher = TRH;

    static constexpr size_t NStartCapacity = 1u;
    static constexpr size_t NLoadRatio = 2u;
    static constexpr double NRehashFactor = 2.0;

    using TStorage = 
        typename std::aligned_storage<sizeof(TData), alignof(TData)>::type;

    CCuckooHashTable() = default;

    template<typename TIter>
    CCuckooHashTable(TIter begin_it, TIter end_it):
        CCuckooHashTable()
    {
        for (TIter iter = begin_it; iter != end_it; ++iter)
        {
            const auto& [key, value] = *iter;
            insert(key, value); // Safe as class is `final`
        }
    }

    virtual ~CCuckooHashTable() final
    {
        size_t cap = capacity();
        for (size_t index = 0u; index < cap * 2u; ++index)
        {
            if (used_vec_[index])
                destruct_at(index);
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
        return data_vec_.size() / 2u;
    }

    [[nodiscard]]
    virtual bool empty() const noexcept override final
    {
        return size_ == 0u;
    }

    virtual bool insert(const TKey& desired, 
                        const TValue& desired_value) override final
    {
        thread_local bool is_in_rehash = false;
        TStorage buffer[2u] = {};

        size_t ratio = get_load_ratio();
        if (capacity() * (ratio - 1) < (size_ + 1) * ratio)
            rehash(capacity() * NRehashFactor);

        if (size_t left_index = left_pos(desired); used_vec_[left_index])
        {
            if (auto& [key, value] = get_data_at(left_index); key == desired)
            {
                value = desired_value;
                return false;
            }
        }

        if (size_t right_index = right_pos(desired); used_vec_[right_index])
        {
            if (auto& [key, value] = get_data_at(right_index); key == desired)
            {
                value = desired_value;
                return false;
            }
        }

        TStorage* storage = buffer;
        TStorage* tmp_storage = buffer + 1u;
        new (storage) TData{ desired, desired_value };

        bool is_left = true;
        size_t base_index = left_pos(desired);
        if (used_vec_[base_index])
        {
            base_index = right_pos(desired);
            is_left = false;
        }

        size_t cur_index = base_index;

        for (size_t count = 0u; (count * 2u) < capacity(); 
             (is_left = !is_left), ++count)
        {
            auto& [key, value] = 
                *std::launder(reinterpret_cast<TData*>(storage));

            if (!used_vec_[cur_index])
            {
                construct_at(cur_index, key, std::move(value));
                used_vec_[cur_index] = true;
                std::launder(reinterpret_cast<TData*>(storage))->~TData();
                ++size_;

                return true;
            }

            auto& [tmp_key, tmp_value] = get_data_at(cur_index);
            size_t next_index = 
                (is_left ? right_pos(tmp_key) : left_pos(tmp_key));

            new (tmp_storage) TData{ tmp_key, std::move(tmp_value) };
            destruct_at(cur_index);

            construct_at(cur_index, key, std::move(value));
            std::launder(reinterpret_cast<TData*>(storage))->~TData();

            std::swap(storage, tmp_storage);

            if (next_index == base_index)
                break;

            cur_index = next_index;
        }

        base_index = 0u;
        while (base_index < data_vec_.size())
        {
            if (!used_vec_[base_index])
                break;

            ++base_index;
        }

        auto& [key, value] = 
            *std::launder(reinterpret_cast<TData*>(storage));
        construct_at(base_index, key, std::move(value));
        used_vec_[base_index] = true;
        ++size_;
        std::launder(reinterpret_cast<TData*>(storage))->~TData();

        if (!is_in_rehash)
        {
            is_in_rehash = true;
            rehash();
            is_in_rehash = false;
        }
        else
        {
            rehash(capacity() * 2u);
        }

        return true;
    }

    virtual bool erase(const TKey& desired) override final
    {
        if (size_t left_index = left_pos(desired); used_vec_[left_index])
        {
            if (auto& [key, value] = get_data_at(left_index); key == desired)
            {
                destruct_at(left_index);
                used_vec_[left_index] = false;
                --size_;

                return true;
            }
        }

        if (size_t right_index = right_pos(desired); used_vec_[right_index])
        {
            if (auto& [key, value] = get_data_at(right_index); key == desired)
            {
                destruct_at(right_index);
                used_vec_[right_index] = false;
                --size_;

                return true;
            }
        }

        return false;
    }

    [[nodiscard]]
    virtual std::optional<std::reference_wrapper<const TValue>> 
        find(const TKey& desired) const override final
    {
        auto result = const_cast<CCuckooHashTable&>(*this).find(desired);

        return (result ? 
                std::make_optional(
                        std::cref(const_cast<TValue&>(result.value().get()))
                    ) : 
                std::nullopt);
    }

    [[nodiscard]]
    virtual std::optional<std::reference_wrapper<TValue>> 
        find(const TKey& desired) override final
    {
        if (size_t left_index = left_pos(desired); used_vec_[left_index])
        {
            if (auto& [key, value] = get_data_at(left_index); key == desired)
                return std::ref(value);
        }

        if (size_t right_index = right_pos(desired); used_vec_[right_index])
        {
            if (auto& [key, value] = get_data_at(right_index); key == desired)
                return std::ref(value);
        }

        return std::nullopt;
    }

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
        return const_cast<CCuckooHashTable*>(this)->get_data_at(idx);
    }

    [[nodiscard]]
    inline TData& get_data_at(size_t idx) noexcept
    {
        return *std::launder(reinterpret_cast<TData*>(&data_vec_[idx]));
    }

    [[nodiscard]]
    inline size_t left_pos(const TKey& desired) const noexcept
    {
        return (left_hasher_(desired) ^ left_xor_) % capacity();
    }

    [[nodiscard]]
    inline size_t right_pos(const TKey& desired) const noexcept
    {
        return (right_hasher_(desired) ^ right_xor_) % capacity() + capacity();
    }

    [[nodiscard]]
    inline size_t get_load_ratio() const noexcept
    {
        return NLoadRatio;
    }

    void rehash()
    {
        thread_local std::random_device seed_dev;
        thread_local std::mt19937 rand_gen(seed_dev());

        TStorage buffer;
        TStorage* storage = &buffer;

        left_xor_ = rand_gen();
        right_xor_ = rand_gen();

        size_t cap = capacity();
        for (size_t index = 0u; index < cap; ++index)
        {
            if (used_vec_[index])
            {
                if (auto& [key_ref, value_ref] = get_data_at(index);
                    left_pos(key_ref) != index)
                {
                    auto& [key, value] = *std::launder(reinterpret_cast<TData*>
                            (new (storage) TData{ key_ref, value_ref }));

                    destruct_at(index);
                    used_vec_[index] = false;
                    --size_;

                    insert(key, std::move(value));
                    std::launder(reinterpret_cast<TData*>(storage))->~TData();
                }
            }

            if (used_vec_[index + cap])
            {
                if (auto& [key_ref, value_ref] = get_data_at(index + cap);
                    right_pos(key_ref) != index + cap)
                {
                    auto& [key, value] = *std::launder(reinterpret_cast<TData*>
                            (new (storage) TData{ key_ref, value_ref }));

                    destruct_at(index + cap);
                    used_vec_[index + cap] = false;
                    --size_;

                    insert(key, std::move(value));
                    std::launder(reinterpret_cast<TData*>(storage))->~TData();
                }
            }
        }
    }

    void rehash(size_t new_capacity)
    {
        size_t old_capacity = capacity();
        if (new_capacity <= old_capacity)
            throw std::invalid_argument(
                    "CCuckooHashTable::rehash(): "
                    "new_capacity <= old_capacity"
                    );

        auto new_data_vec = std::vector<TStorage>(new_capacity * 2u);
        for (size_t index = 0u; index < old_capacity * 2u; ++index)
        {
            if (used_vec_[index])
            {
                auto& [key, value] = get_data_at(index);
                new (&new_data_vec[index]) TData{ key, std::move(value) };
                destruct_at(index);
            }
        }

        used_vec_.resize(new_capacity * 2u, false);
        data_vec_ = std::move(new_data_vec);

        rehash();
    }

private:
    size_t size_{};

    size_t left_xor_{ static_cast<size_t>(-1) };
    size_t right_xor_{};

    TLeftHasher left_hasher_{};
    TRightHasher right_hasher_{};

    std::vector<bool> used_vec_ = std::vector<bool>(NStartCapacity * 2, false);
    std::vector<TStorage> data_vec_ = std::vector<TStorage>(NStartCapacity * 2);
};

} // namespace

#endif // CUCKOO_HASHTABLE_H_
