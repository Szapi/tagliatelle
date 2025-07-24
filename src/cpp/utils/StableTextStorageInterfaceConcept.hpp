#pragma once

#include <concepts>
#include <string_view>

namespace tagliatelle::concepts
{

    namespace _detail
    {

        template<class T>
        concept StableTextStorageInterface = requires (T& t, std::string_view sv)
        {
            { t.Store(sv) } -> std::same_as<std::string_view>;
            requires std::copyable<T>;
        };

    }

    template<class T, class ... Us>
    concept StableTextStorageInterface = (_detail::StableTextStorageInterface<T> && _detail::StableTextStorageInterface<Us> && ...);

} // namespace tagliatelle::concepts
