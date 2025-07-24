#pragma once

#include <vector>
#include <string>
#include <string_view>

#include "StableTextStorageInterfaceConcept.hpp"
#include "Utils.hpp"

namespace tagliatelle
{

    class TrivialTextStorage
    {
    public:
        TrivialTextStorage() = default;

        MOVE_ONLY(TrivialTextStorage);

        [[nodiscard]] auto Store(std::string_view str) -> std::string_view
        {
            return storage.emplace_back(str);
        }

        // Basically a delegate
        class StorageInterface
        {
        public:
            StorageInterface(TrivialTextStorage& impl) : impl {&impl} {}
            COPYABLE(StorageInterface);

            [[nodiscard]] std::string_view Store(std::string_view str)
            {
                return impl->Store(str);
            }

        private:
            TrivialTextStorage* impl;
        };

        static_assert(concepts::StableTextStorageInterface<StorageInterface>);
        
        [[nodiscard]] StorageInterface GetInterface() { return {*this}; }

    private:
        std::vector<std::string> storage;
    };
    
} // namespace tagliatelle
