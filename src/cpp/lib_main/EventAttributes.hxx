#include "Utils.hpp"

namespace tagliatelle
{
    
    enum class AttributeKind
    {
        Timestamp,
        Text,
        DynamicEnum,
        StaticEnum,
        Number
    };

    namespace _detail
    {
        template<AttributeKind Kind, typename Tested>
        constexpr bool IsKind = std::is_same_v<Tested, std::integral_constant<AttributeKind, Kind>>;
    };

    template<class T>
    concept EventAttribute = requires ()
    {
        requires _detail::IsIntegralConstant<typename T::Kind,        AttributeKind>;
        requires _detail::IsIntegralConstant<typename T::IsTiebraker, bool>;
    };

    template<class T>
    concept TiebrakerAttribute = requires ()
    {
        requires EventAttribute<T>;
        requires std::is_same_v<std::true_type, typename T::IsTiebraker>;
        requires _detail::IsIntegralConstant<typename T::TiebrakerRank, std::size_t>;
    };

    #define NOT_TIEBRAKER using IsTiebraker = std::false_type

    #define TIEBRAKER(rank)                     \
        using IsTiebraker   = std::true_type;   \
        using TiebrakerRank = std::integral_constant<std::size_t, rank>


///////////////////////////////////////////////////////////////////////////
// TextAttribute

    enum class LongTextHandlingStrategy
    {
        Truncate,
        Keep
    };

    template<class T>
    concept TextAttribute = requires()
    {
        requires EventAttribute<T>;
        requires _detail::IsKind<AttributeKind::Text, typename T::Kind>;
        requires _detail::IsIntegralConstant<typename T::PageSize,         std::size_t>;
        requires _detail::IsIntegralConstant<typename T::MaxLength,        std::size_t>;
        requires _detail::IsIntegralConstant<typename T::LongTextHandling, LongTextHandlingStrategy>;
        requires T::PageSize::value >= T::MaxLength::value;
        requires T::MaxLength::value > 0;
    };

    #define DEFINE_TEXT_PART(name, maxLen, pageSize, longTextHandling)                  \
        class name final { public:                                                      \
            NOT_TIEBRAKER;                                                              \
            using Kind = std::integral_constant<AttributeKind, AttributeKind::Text>;    \
            using MaxLength = std::integral_constant<std::size_t, maxLen>;              \
            using PageSize  = std::integral_constant<std::size_t, pageSize>;            \
            using LongTextHandling  = std::integral_constant<LongTextHandlingStrategy, LongTextHandlingStrategy:: longTextHandling>; \
        }

///////////////////////////////////////////////////////////////////////////
// StaticEnumAttribute

    template<class T>
    concept StaticEnumAttribute = requires()
    {
        requires EventAttribute<T>;
        requires _detail::IsKind<AttributeKind::StaticEnum, typename T::Kind>;
        requires _detail::AreUnique(T::Values);
    };

    #define DEFINE_STATIC_ENUM_PART(name, values)                                           \
        class name final { public:                                                          \
            NOT_TIEBRAKER;                                                                  \
            using Kind = std::integral_constant<AttributeKind, AttributeKind::StaticEnum>;  \
            inline static constexpr auto Values = _detail::SortStrings values ;             \
        }

///////////////////////////////////////////////////////////////////////////
// NumberAttribute

    template<class T>
    concept NumberAttribute = requires()
    {
        requires EventAttribute<T>;
        requires _detail::IsKind<AttributeKind::Number, typename T::Kind>;
        requires std::is_arithmetic_v<typename T::Representation>;
    };

    #define DEFINE_NUMBER_PART(name, representation, tiebraker)                         \
        class name final { public:                                                      \
            tiebraker;                                                                  \
            using Kind = std::integral_constant<AttributeKind, AttributeKind::Number>;  \
            using Representation = representation;                                      \
        }

///////////////////////////////////////////////////////////////////////////
// TimestampAttribute

    template<class T>
    concept TimestampAttribute = requires()
    {
        requires _detail::IsKind<AttributeKind::Timestamp, typename T::Kind>;
        requires TiebrakerAttribute<T>;
    };

    #define DEFINE_TIMESTAMP_PART(name, tiebrakerRank)                                      \
        class name final { public:                                                          \
            TIEBRAKER(tiebrakerRank);                                                       \
            using Kind = std::integral_constant<AttributeKind, AttributeKind::Timestamp>;   \
        }

///////////////////////////////////////////////////////////////////////////
// DynamicEnumAttribute

    template<class T>
    concept DynamicEnumAttribute = requires()
    {
        requires _detail::IsKind<AttributeKind::DynamicEnum, typename T::Kind>;
        requires _detail::IsIntegralConstant<typename T::MaxLength,        std::size_t>;
        requires _detail::IsIntegralConstant<typename T::PageSize,         std::size_t>;
        requires _detail::IsIntegralConstant<typename T::LongTextHandling, LongTextHandlingStrategy>;
        requires T::PageSize::value >= T::MaxLength::value;
        requires T::MaxLength::value > 0;
    };

    #define DEFINE_DYNAMIC_ENUM_PART(name, maxLen, pageSize, longTextHandling)                  \
        class name final { public:                                                              \
            NOT_TIEBRAKER;                                                                      \
            using Kind = std::integral_constant<AttributeKind, AttributeKind::DynamicEnum>;     \
            using MaxLength        = std::integral_constant<std::size_t, maxLen>;               \
            using PageSize         = std::integral_constant<std::size_t, pageSize>;             \
            using LongTextHandling = std::integral_constant<LongTextHandlingStrategy, LongTextHandlingStrategy:: longTextHandling>; \
        }

///////////////////////////////////////////////////////////////////////////
// EventDefinition

    namespace _detail
    {

        template<class...Attributes>
        constexpr std::size_t TimestampCount = ((TimestampAttribute<Attributes> ? 1 : 0) + ...);

        template<class...Attributes>
        constexpr std::size_t TiebrakerCount = ((TiebrakerAttribute<Attributes> ? 1 : 0) + ...);
        
        template<class...Attributes>
        consteval std::size_t NthTiebrakerRank(const std::size_t n)
        {
            std::size_t rank;
            bool found = false;
            auto Check = [&, num = 0]<class P>()mutable
            {
                if constexpr(TiebrakerAttribute<P>)
                {
                    if (n == num)
                    {
                        rank = P::TiebrakerRank::value;
                        found = true;
                    }
                    ++num;
                }
            };

            (Check.template operator()<Attributes>(), ...);
            if (!found) throw;
            return rank;
        }
        
        template<class...Attributes>
        consteval bool UnambiguousTiebrakers()
        {
            std::array<std::size_t, TiebrakerCount<Attributes...>> arr;
            for (int i = 0; i < arr.size(); ++i)
            {
                arr[i] = NthTiebrakerRank<Attributes...>(i);
            }
            std::ranges::sort(arr);
            return std::adjacent_find(arr.cbegin(), arr.cend(), [](const auto a, const auto b){ return a == b; }) == arr.cend();
        }

    } // namespace _detail

    template<class...Attributes>
    concept EventDefinition = requires ()
    {
        requires _detail::TimestampCount<Attributes...> == 1;
        requires _detail::UnambiguousTiebrakers<Attributes...>();
    };

} // namespace tagliatelle
