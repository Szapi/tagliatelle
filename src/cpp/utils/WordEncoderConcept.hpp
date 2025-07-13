#pragma once

#include <concepts>
#include <string_view>

namespace tagliatelle::concepts
{

    template<class T>
    concept WordEncoderInterface = requires (T& t, std::string_view sv)
    {
        typename T::EncodedType;
        { t.Encode(sv) } -> std::same_as<typename T::EncodedType>;
    };

} // namespace tagliatelle::concepts
