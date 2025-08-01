#pragma once

#include "EventAttributes.hxx"
#include "AttributeCodec.hxx"
#include "PagedArray.hpp"
#include "Permit.hpp"

namespace tagliatelle
{

    // The Event Pool is the main data structure that holds all known events.
    // Events are encoded for efficient storage and filtering.
    template<class...Attrs>
        requires EventDefinition<Attrs...>
    class EventPool
    {
        SINGLETON(EventPool);

        struct UniqueID { std::size_t value; };
        struct SourceID { std::size_t value; };

        using EncodedEvent = std::tuple<
            UniqueID,
            SourceID,
            typename Codec<Attrs>::EncodedType
            ...
        >;
        static_assert(std::is_trivial_v<EncodedEvent>);

    public:
        class SourceToken
        {
        public:
            SourceToken(SourceID id, Permit<EventPool<Attrs...>>) : id{id} {}

            // Copyable
            SourceToken(const SourceToken&) = default;
            SourceToken& operator=(const SourceToken&) = default;

            // Moveable
            SourceToken(SourceToken&&) = default;
            SourceToken& operator=(SourceToken&&) = default;

            // Comparable
            auto operator<=>(const SourceToken&) const = default;

            SourceID GetID(Permit<EventPool<Attrs...>>) const { return id; }
            
        private:
            SourceID id;
        };

        // Not thread-safe!
        [[nodiscard]] SourceToken RegisterEventSource(const std::string_view info)
        {
            SourceToken token{registeredEventSources.Size(), Permit{this}};
            registeredEventSources.Emplace(eventSourceInfoTexts.Store(info));
            return token;
        }

    private:
        inline static constexpr std::size_t EventSourceString_SizeThreshold  = 256u;
        inline static constexpr std::size_t EventSourceString_BufferPageSize = 4096u;

        PagedArray<std::string_view, 128u> registeredEventSources{};
        _detail::SizeSensitiveTextStorage<
            EventSourceString_SizeThreshold,
            EventSourceString_BufferPageSize
        > eventSourceInfoTexts;
    };

} // namespace tagliatelle
