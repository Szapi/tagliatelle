#pragma once

#include <concepts>
#include <limits>
#include <optional>
#include <unordered_map>
#include <vector>

#include "StableTextStorageInterfaceConcept.hpp"
#include "Utils.hpp"
#include "Permit.hpp"

namespace tagliatelle
{

    template<
        std::integral EncodeType,
        concepts::StableTextStorageInterface TextStorage,
        EncodeType MaxValue = std::numeric_limits<EncodeType>::max()>
    >
    class WordDictionary
    {
        inline static constexpr EncodeType EmptyEncoded  = 0; // Reserved for empty string
        inline static constexpr EncodeType StartingValue = 1;

        inline static constexpr auto DictionaryFullString = std::string_view{"!DICTIONARY FULL!"};

    public:
        WordDictionary(TextStorage textStorage) : texts{textStorage} {}
        
        // Helper constructor for CTAD
        WordDictionary(_detail::With<EncodeType>,
                       TextStorage textStorage,
                       _detail::Constant<EncodeType, MaxValue>) :
            WordDictionary(textStorage)
        {}

        MOVE_ONLY(WordDictionary);

        EncodeType Encode(const std::string_view str)
        {
            if (str.empty())
                return EmptyEncoded;

            if (MaxValue == nextValue) [[unlikely]]
                return TryEncode(str).value_or(MaxValue);

            const auto [it, inserted] = encodeMap.try_emplace(str, nextValue);
            if (inserted)
            {
                const_cast<std::string_view&>(it->first) = texts.Store(str);
                decodeMap.emplace_back(it->first);
                ASSERT(str == Decode(nextValue), "WordDictionary: encoding error");
                ++nextValue;
                if (MaxValue == nextValue)
                    decodeMap.emplace_back(DictionaryFullString);
            }

            return it->second;
        }

        std::optional<EncodeType> TryEncode(const std::string_view str)
        {
            if (str.empty())
                return EmptyEncoded;

            const auto it = encodeMap.find(str);
            if (it != encodeMap.end())
                return it->second;

            return std::nullopt;
        }

        std::string_view Decode(const EncodeType encoded) const
        {
            if (!_detail::ValidIndexZB(encoded, decodeMap)) [[unlikely]]
                return "!OUT OF RANGE!";

            return decodeMap.at(static_cast<std::size_t>(encoded));
        }


        class Expander
        {
            friend class WordDictionary;
        public:
            Expander(WordDictionary& _master)
                : master{ &_master }
            {
                nextValue = _master.GetNextValue(Permit{ this });
            }

            MOVE_ONLY(Expander);

            EncodeType Encode(const std::string_view str)
            {
                if (const auto attempt = master->TryEncode(str); attempt.has_value())
                    return *attempt;

                if (nextValue == MaxValue)
                {
                    const auto it = encodeMap.find(str);
                    return it != encodeMap.end() ? it->second : MaxValue;
                }

                const auto [it, inserted] = encodeMap.try_emplace(str, nextValue);
                if (inserted)
                {
                    const_cast<std::string_view&>(it->first) = master->GetStorage(Permit{ this }).Store(str);
                    ++nextValue;
                }
                return it->second;
            }

        private:
            WordDictionary* master;
            EncodeType      nextValue;
            std::unordered_map<std::string_view, EncodeType> encodeMap;
        };

        Expander GetExpander()
        {
            return { *this };
        }

        bool Merge(Expander&& expander)
        {
            ASSERT(this == expander.master, "Trying to merge foreign dictionary expander");
            ASSERT(decodeMap.size() + expander.encodeMap.size() == expander.nextValue, "Expander item count error");

            if (expander.encodeMap.empty())
                return false;

            // Update nextValue
            nextValue = expander.nextValue;

            // Update decodeMap
            decodeMap.resize(nextValue);
            for (const auto [str, encoded] : expander.encodeMap)
            {
                ASSERT(_detail::ValidIndexZB(encoded, decodeMap) && encoded != MaxValue, "Expander item out of range");
                decodeMap.at(static_cast<std::size_t>(encoded)) = str;
            }
            if (MaxValue == nextValue)
                decodeMap.emplace_back(DictionaryFullString);

            // Merge encode maps
            this->encodeMap.merge(std::move(expander.encodeMap));

            return true;
        }

        TextStorage GetStorage(Permit<Expander>)
        {
            return texts;
        }
        EncodeType  GetNextValue(Permit<Expander>)
        {
            return nextValue;
        }

    private:
        TextStorage texts;
        EncodeType  nextValue = StartingValue;

        std::unordered_map<std::string_view, EncodeType> encodeMap;
        std::vector<std::string_view>                    decodeMap;
    };

} // namespace tagliatelle
