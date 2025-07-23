#pragma once

#include <algorithm>
#include <array>
#include <ranges>
#include <string_view>
#include <type_traits>

#include "DebugMacros.hpp"


#define IMMOVABLE(Class)            \
    Class(const Class&) = delete;   \
    Class& operator=(const Class&) = delete


#define COPYABLE(Class)             \
    Class(const Class&) = default;  \
    Class& operator=(const Class&) = default


#define MOVE_ONLY(Class)            \
    IMMOVABLE(Class);               \
    Class(Class&&) = default;       \
    Class& operator=(Class&&) = default

    
namespace tagliatelle::_detail
{

    template<class> struct With{};

    consteval bool IsPowerOfTwo(const std::size_t n)
    {
        return (n > 1) && ((n & (n - 1)) == 0);
    }

    // Valid zero-based index for random-access container
    constexpr bool ValidIndexZB(const auto idxUntyped, const auto& container)
    {
        const auto idx = static_cast<std::size_t>(idxUntyped);
        return (idx >= 0) && (idx < container.size());
    }

    // This trait can be used to detect if an expression is in fact a compile-time constant
    template<class,class>    constexpr bool IsIntegralConstant = false;
    template<class T, T val> constexpr bool IsIntegralConstant<T,std::integral_constant<T, val>> = true;

    // Compile-time operations on string_view arrays
    template<std::size_t N>
    consteval bool AreUnique(const std::array<std::string_view, N>& arr)
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            for (std::size_t j = 0; j < N; ++j)
            {
                if (i == j) continue;
                if (arr[i] == arr[j]) return false;
            }
        }
        return true;
    }

    consteval auto SortStrings(auto&&... strs)
    {
        std::array<std::string_view, sizeof...(strs)> result{strs...};
        std::ranges::sort(result);
        return result;
    }

} // namespace tagliatelle::_detail