#pragma once

#include "StableTextBuffer.hpp"

namespace tagliatelle
{

    template<std::size_t MaxSize, std::size_t PageSize> requires (MaxSize <= PageSize)
    class TruncatingTextStorage
    {
    public:
        TruncatingTextStorage() = default;
        
        [[nodiscard]] auto Store(const auto& str) -> std::string_view
        {
            return storage.Store(
                std::string_view{str.data(), std::min(str.length(), MaxSize)}
            );
        }

        // Basically a delegate
        class StorageInterface
        {
        public:
            StorageInterface(TruncatingTextStorage& impl) : impl {&impl} {}
            COPYABLE(StorageInterface);

            [[nodiscard]] auto Store(const auto& str) -> std::string_view
            {
                return impl->Store(str);
            }

        private:
            TruncatingTextStorage* impl;
        };

        static_assert(concepts::StableTextStorageInterface<StorageInterface>);
        
        [[nodiscard]] StorageInterface GetInterface() { return {*this}; }

    private:
        StableTextBuffer<PageSize> storage;
    };
    
} // namespace tagliatelle
