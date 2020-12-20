#ifndef HASHER_ADAPTER_H_
#define HASHER_ADAPTER_H_

#include <cstdint>
#include <string_view>

namespace {

template<typename THasher>
class CHasherAdapter
{
public:
    CHasherAdapter() = default;

    explicit CHasherAdapter(const THasher& hasher):
        hasher_(hasher)
    {}

    explicit CHasherAdapter(THasher&& hasher):
        hasher_(std::move(hasher))
    {}

    [[nodiscard]]
    size_t operator()(size_t key) const
    {
        return hasher_(reinterpret_cast<const uint8_t*>(&key), 
                       sizeof(key));
    }

    [[nodiscard]]
    size_t operator()(std::string_view key) const
    {
        return hasher_(reinterpret_cast<const uint8_t*>(key.data()), 
                       key.size());
    }

    template<typename TKey>
    [[nodiscard]]
    size_t operator()(const TKey& key) const
    {
        return hasher_(reinterpret_cast<const uint8_t*>(&key), 
                       sizeof(key));
    }

private:
    THasher hasher_;
};

} // namespace

#endif // HASHER_ADAPTER_H_
