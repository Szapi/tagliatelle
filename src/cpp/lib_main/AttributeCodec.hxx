
#include <chrono>
#include <string>
#include <vector>

#include "EventAttributes.hxx"

namespace tagliatelle
{

    template<class> class Codec {};

///////////////////////////////////////////////////////////////////////////
// TextAttribute

    template<class T> requires TextAttribute<T>
    class Codec<T>
    {
    public:
        using EncodedType = std::string_view;

        EncodedType Encode(const std::string_view str)
        {
            if (str.length() > T::MaxLength::value)
            {
                if constexpr (T::LongTextHandling::value == LongTextHandlingStrategy::Truncate)
                {
                    return Encode(std::string_view{str.data(), T::MaxLength::value});
                }
                if constexpr (T::LongTextHandling::value == LongTextHandlingStrategy::Keep)
                {
                    return longTextStorage.emplace_back(str);
                }
            }

            return regularStorage.emplace_back(str);
        }

        std::string_view Decode(const EncodedType str)
        {
            return str;
        }

    private:
        std::vector<std::string> regularStorage; // TODO
        std::vector<std::string> longTextStorage;
    };
    
///////////////////////////////////////////////////////////////////////////
// StaticEnumAttribute
    
    template<class T> requires StaticEnumAttribute<T>
    class Codec<T>
    {
    public:
        using EncodedType = std::size_t;

        constexpr EncodedType Encode(const std::string_view str)
        {
            static_assert(std::ranges::is_sorted(T::Values));
            const auto it = std::ranges::lower_bound(T::Values, str);
            if (it != T::Values.cend() && *it == str) [[likely]]
                return std::distance(T::Values.cbegin(), it);

            return T::Values.size();
        }

        constexpr std::string_view Decode(const EncodedType n)
        {
            if (n >= T::Values.size())
                return "!INVALID ENUM!";

            return T::Values[n];
        }
    };
    
///////////////////////////////////////////////////////////////////////////
// TimestampAttribute

    using TimestampType = std::chrono::system_clock::time_point;

    template<class T> requires TimestampAttribute<T>
    class Codec<T>
    {
    public:
        using EncodedType = TimestampType;

        constexpr EncodedType Encode(const TimestampType ts)
        {
            return ts;
        }

        constexpr TimestampType Decode(const EncodedType ts)
        {
            return ts;
        }
    };

} // namespace tagliatelle
