#pragma once

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

} // namespace tagliatelle::_detail