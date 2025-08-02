
#include <chrono>
#include <string>
#include <vector>

#include "EventAttributes.hxx"
#include "StableTextBuffer.hpp"
#include "TrivialTextStorage.hpp"

namespace tagliatelle
{

    // Summary: The Codec<T> classes encode the different parts (attributes) of the event as POD
    //          (trivial) types for efficient storage and filtering.
    // Contract: Decoding events (for the UI or filtering) can be done without mutex locking,
    //           even while the codec is being used for encoding newly acquired events.
    // Contract: UI receives decoded events as a set of string_view's, in a zero-copy fashion,
    //           which must remain valid until program end or a full clear.
    template<class> class Codec {};

///////////////////////////////////////////////////////////////////////////
// TextAttribute

    template<class T> requires TextAttribute<T>
    class Codec<T>
    {
    public:
        using EncodedType = std::string_view;

        static_assert(std::is_trivial_v<EncodedType>);

        [[nodiscard]] auto Encode(const auto& str) -> EncodedType
        {
            return storage.Store(str);
        }

        [[nodiscard]] auto Decode(const EncodedType str) const -> std::string_view
        {
            return str;
        }

    private:
        // Stable text buffers allow simultaneous encoding/decoding
        std::conditional_t<
            T::LongTextHandling::value == LongTextHandlingStrategy::Keep,
            _detail::SizeSensitiveTextStorage<T::MaxLength::value, T::PageSize::value>,
            _detail::TruncatingTextStorage<T::MaxLength::value, T::PageSize::value>
        > storage;
    };
    
///////////////////////////////////////////////////////////////////////////
// StaticEnumAttribute
    
    template<class T> requires StaticEnumAttribute<T>
    class Codec<T>
    {
    public:
        using EncodedType = std::size_t;
        
        static_assert(std::is_trivial_v<EncodedType>);

        // Technically a compile-time construct, so parallel encoding/decoding is safe.
        // Nice-to-have in the future: Perfect hashing (e.g. gperf)
        [[nodiscard]] constexpr auto Encode(const std::string_view str) -> EncodedType
        {
            static_assert(std::ranges::is_sorted(T::Values));
            const auto it = std::ranges::lower_bound(T::Values, str);
            if (it != T::Values.cend() && *it == str) [[likely]]
                return std::distance(T::Values.cbegin(), it);

            return T::Values.size();
        }

        [[nodiscard]] constexpr auto Decode(const EncodedType n) const -> std::string_view
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
        
        static_assert(std::is_trivial_v<EncodedType>);

        [[nodiscard]] constexpr auto Encode(const TimestampType ts) -> EncodedType
        {
            return ts;
        }

        [[nodiscard]] constexpr auto Decode(const EncodedType ts) const -> TimestampType
        {
            return ts;
        }
    };
    
///////////////////////////////////////////////////////////////////////////
// DynamicEnumAttribute

    template<class T> requires DynamicEnumAttribute<T>
    class Codec<T>
    {
    public:
        using EncodedType = std::size_t;
        
        static_assert(std::is_trivial_v<EncodedType>);

        [[nodiscard]] auto Encode(const std::string_view str) -> EncodedType
        {
            if constexpr (T::LongTextHandling::value == LongTextHandlingStrategy::Truncate)
            {
                if (str.length() > T::MaxLength::value)
                {
                    return Encode(
                        std::string_view{str.data(), T::MaxLength::value}
                    );
                }
            }

            return dictionary.Encode(str);
        }

        [[nodiscard]] auto Decode(const EncodedType n) const -> std::string_view
        {
            dictionary.Decode(str);
        }
    
    private:
        std::conditional_t<
            T::LongTextHandling::value == LongTextHandlingStrategy::Keep,
            _detail::SizeSensitiveTextStorage<T::MaxLength::value, T::PageSize::value>,
            _detail::TruncatingTextStorage<T::MaxLength::value, T::PageSize::value>
        > textStorage;

        WordDictionary dictionary{_detail::With<EncodedType>{}, textStorage.GetInterface()};
    };
    
///////////////////////////////////////////////////////////////////////////
// NumberAttribute

    template<class T> requires NumberAttribute<T>
    class Codec<T>
    {
    public:
        using RawType     = typename T::Representation;
        using EncodedType = Raw;
        
        static_assert(std::is_trivial_v<EncodedType>);

        [[nodiscard]] constexpr auto Encode(const RawType r) -> EncodedType
        {
            return r;
        }

        [[nodiscard]] constexpr auto Decode(const EncodedType e) const -> RawType
        {
            return e;
        }
    };

} // namespace tagliatelle
