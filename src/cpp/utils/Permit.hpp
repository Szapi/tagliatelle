#pragma once

#include <type_traits>

namespace tagliatelle
{

    template<class T>
    class Permit
    {
        static_assert(std::is_same_v<T, std::remove_cvref_t<T>>);
        friend T;
        Permit(const T*) {}
    };
    
} // namespace tagliatelle
