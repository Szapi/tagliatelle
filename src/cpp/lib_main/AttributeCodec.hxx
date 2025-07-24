
#include <chrono>
#include <string>
#include <vector>

#include "EventAttributes.hxx"
#include "StableTextBuffer.hpp"
#include "TrivialTextStorage.hpp"

namespace tagliatelle
{

    template<class> class Codec {};

    namespace _detail
    {

        template<std::size_t Threshold, std::size_t PageSize> requires (Threshold <= PageSize)
        class SizeSensitiveTextStorage
        {
        public:
            SizeSensitiveTextStorage() = default;
            
            [[nodiscard]] auto Store(const auto& str) -> std::string_view
            {
                return str.length() > Threshold ? longStorage.Store(str) : shortStorage.Store(str);
            }

            // Basically a delegate
            class StorageInterface
            {
            public:
                StorageInterface(SizeSensitiveTextStorage& impl) : impl {&impl} {}
                COPYABLE(StorageInterface);

                [[nodiscard]] auto Store(const auto& str) -> std::string_view
                {
                    return impl->Store(str);
                }

            private:
                SizeSensitiveTextStorage* impl;
            };

            static_assert(concepts::StableTextStorageInterface<StorageInterface>);
            
            [[nodiscard]] StorageInterface GetInterface() { return {*this}; }

        private:
            StableTextBuffer<PageSize> shortStorage;
            TrivialTextStorage longStorage;
        };

        template<std::size_t MaxSize, std::size_t PageSize> requires (MaxSize <= PageSize)
        class TruncatingTextStorage
        {
        public:
            TruncatingTextStorage() = default;
            
            [[nodiscard]] auto Store(const auto& str) -> std::string_view
            {
                return storage.Store(
                    std::string_view{str.data(), std::min(str.length(), MaxSize)}
                );
            }

        private:
            StableTextBuffer<PageSize> storage;
        };
        
    } // namespace _detail
    

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
