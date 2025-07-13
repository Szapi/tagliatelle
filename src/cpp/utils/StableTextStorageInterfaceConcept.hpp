#pragma once

#include <concepts>
#include <string_view>

namespace tagliatelle::concepts
{

    template<class T>
    concept StableTextStorageInterface = requires (T& t, std::string_view sv)
    {
        { t.Store(sv) } -> std::same_as<std::string_view>;
        requires std::copyable<T>;
    };

} // namespace tagliatelle::concepts
