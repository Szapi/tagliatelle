#pragma once

#include "StableTextBuffer.hpp"
#include "TrivialTextStorage.hpp"

namespace tagliatelle
{

    template<std::size_t Threshold, std::size_t PageSize> requires (Threshold <= PageSize)
    class SizeSensitiveTextStorage
    {
    public:
        SizeSensitiveTextStorage() = default;
        
        [[nodiscard]] auto Store(const auto& str) -> std::string_view
        {
            return str.length() > Threshold ? longStorage.Store(str) : shortStorage.Store(str);
        }

        // Basically a delegate
        class StorageInterface
        {
        public:
            StorageInterface(SizeSensitiveTextStorage& impl) : impl {&impl} {}
            COPYABLE(StorageInterface);

            [[nodiscard]] auto Store(const auto& str) -> std::string_view
            {
                return impl->Store(str);
            }

        private:
            SizeSensitiveTextStorage* impl;
        };

        static_assert(concepts::StableTextStorageInterface<StorageInterface>);
        
        [[nodiscard]] StorageInterface GetInterface() { return {*this}; }

    private:
        StableTextBuffer<PageSize> shortStorage;
        TrivialTextStorage longStorage;
    };
    
} // namespace tagliatelle
