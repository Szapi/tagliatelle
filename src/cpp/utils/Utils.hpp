#pragma once

#include <algorithm>
#include <array>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <tuple>

#include "DebugMacros.hpp"

// ----------------------------------------------------------------
// Class traits

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


#define SINGLETON(Class)            \
    public: IMMOVABLE(Class);       \
    inline static Class& Instance() { static Class inst{}; return inst; } \
    private: Class() = default

    
namespace tagliatelle::_detail
{

// ----------------------------------------------------------------
// Template metaprogramming utilities

    template<class> struct With{};

    template<class T, T> struct Constant {};
    
    // This trait can be used to detect if an expression is in fact a compile-time constant
    template<class,class>    constexpr bool IsIntegralConstant = false;
    template<class T, T val> constexpr bool IsIntegralConstant<T,std::integral_constant<T, val>> = true;

// ----------------------------------------------------------------
// Mathematical utilities

    consteval bool IsPowerOfTwo(const std::size_t n)
    {
        return (n >= 1) && ((n & (n - 1)) == 0);
    }

    // Valid zero-based index for random-access container
    constexpr bool ValidIndexZB(const auto idxUntyped, const auto& container)
    {
        const auto idx = static_cast<std::size_t>(idxUntyped);
        return (idx >= 0) && (idx < container.size());
    }

// ----------------------------------------------------------------
// Parameter pack traits

    template<class T, class...Pack>
    concept UniqueInPack = requires
    {
        std::get<T>(std::declval<std::tuple<Pack...>>());
    };

    template<class...Pack>
    constexpr bool UniquePack = (UniqueInPack<Pack, Pack...> && ...);

    // Zero-based index of a type in a pack
    template<class T, class...Pack>
    consteval std::size_t _PackIndexImpl()
    {
        static_assert(sizeof...(Pack) > 0, "PackIndex requires at least one type in the pack");
        static_assert(UniquePack<Pack...>, "PackIndex requires unique types in the pack");
        static_assert(UniqueInPack<T, Pack...>, "PackIndex requires T to be in the pack");

        bool found = false;
        auto index = std::size_t{0};
        auto countUntilT = [&]<class U>(With<U>)
        {
            if constexpr (std::is_same_v<T, U>)
                found = true;
            else
                index += found ? 0 : 1;
        };
        (countUntilT(With<Pack>{}), ...);
        if (!found)
            throw "PackIndex: Type not found in pack";
        return index;
    }

    template<class T, class...Pack>
    consteval std::size_t PackIndex()
    {
        // Validate the result at compile time
        static_assert(
            std::is_same_v<
                T,
                std::tuple_element_t<_PackIndexImpl<T, Pack...>(), std::tuple<Pack...>>
            >,
            "PackIndex internal error: type mismatch"
        );
        return _PackIndexImpl<T, Pack...>();
    }
    
// ----------------------------------------------------------------
// Compile-time string operations

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