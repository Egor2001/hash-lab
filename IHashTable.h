#ifndef IHASHTABLE_H_
#define IHASHTABLE_H_

#include <list>
#include <utility>
#include <optional>
#include <functional>

namespace {

template<class TK, class TV>
class IHashTable
{
public:
    using TKey = const std::remove_cv_t<std::remove_reference_t<TK>>;
    using TValue = TV;
    using TData = std::pair<TKey, TValue>;

    IHashTable() = default;

    IHashTable             (const IHashTable&) = default;
    IHashTable& operator = (const IHashTable&) = default;
    IHashTable             (IHashTable&&) noexcept = default;
    IHashTable& operator = (IHashTable&&) noexcept = default;

    virtual ~IHashTable() = default;

    [[nodiscard]] virtual size_t size() const noexcept = 0;
    [[nodiscard]] virtual size_t capacity() const noexcept = 0;
    [[nodiscard]] virtual bool empty() const noexcept = 0;

    virtual bool insert(const TKey&, const TValue&) = 0;
    virtual bool erase(const TKey&) = 0;

    [[nodiscard]]
    virtual std::optional<std::reference_wrapper<TValue>> 
        find(const TKey&) = 0;

    [[nodiscard]]
    virtual std::optional<std::reference_wrapper<const TValue>> 
        find(const TKey&) const = 0;
};

template<class TK, class TV>
bool IHashTable<TK, TV>::empty() const noexcept
{
    return size() == 0u;
}

} // namespace

#endif // IHASHTABLE_H_
