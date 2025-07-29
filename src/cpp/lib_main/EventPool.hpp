#pragma once

#include "EventAttributes.hxx"
#include "AttributeCodec.hxx"
#include "PagedArray.hpp"

namespace tagliatelle
{
    
    // The Event Pool is the main data structure that holds all known events.
    // Events are encoded for efficient storage and filtering.
    template<class...Attrs>
        requires EventDefinition<Attrs...>
    class EventPool
    {
        struct UniqueID { std::size_t value; };
        struct SourceID { std::size_t value; };

        using EncodedEvent = std::tuple<
            UniqueID,
            SourceID,
            typename Codec<Attrs>::EncodedType
            ...
        >;
        static_assert(std::is_trivial_v<EncodedEvent>);
    };

} // namespace tagliatelle
