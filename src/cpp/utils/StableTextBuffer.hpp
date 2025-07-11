#pragma once

#include <algorithm> // std::copy_n
#include <array>
#include <forward_list>
#include <string_view>

#include "Utils.hpp"

namespace tagliatelle
{

    template <std::size_t PageSz>
    class StableTextBuffer
    {

        class Page
        {
        public:
            bool Empty() const
            {
                return occupied == 0;
            }

            bool CanFit(const std::string_view str) const
            {
                const auto remainingSpace = PageSz - occupied;
                return str.size() <= remainingSpace;
            }

            std::string_view StoreUnsafe(const std::string_view str)
            {
                ASSERT(CanFit(str), _F("StoreUnsafe: cannot fit string of length {}, remaining space is {}, page size is {}", str.size(), PageSz - occupied, PageSz));

                const auto len = str.length();
                const auto dst = data.begin() + occupied;
                std::copy_n(str.begin(), len, dst);
                occupied += len;
                return std::string_view{ std::addressof(*dst), len };
            }

            void Recycle()
            {
                occupied = 0;
            }

            IMMOVABLE(Page);

        private:
            std::array<char, PageSz> data;
            std::size_t occupied = 0;
        };

    public:
        StableTextBuffer() = default;

        MOVE_ONLY(StableTextBuffer);

        void Clear()
        {
            pages.clear();
        }

        void Recycle()
        {
            for (auto& p : pages) p.Recycle();
        }

        std::string_view Store(const std::string_view str)
        {
            ASSERT(str.size() <= PageSz, _F("StableTextBuffer: cannot fit string of length {}, page size is only {}", str.size(), PageSz));

            if (str.empty()) [[unlikely]]
                return str;

            if (pages.empty()) [[unlikely]]
                return pages.emplace_front().StoreUnsafe(str);

            if (pages.front().CanFit(str)) [[likely]]
                return pages.front().StoreUnsafe(str);

            // Check if recycled
            const auto secondPage = ++pages.cbegin();
            if (secondPage != pages.cend() && secondPage->Empty())
            {
                // Effectively rotates the list once to the left
                pages.splice_after(pages.before_begin(), pages, pages.cbegin(), pages.cend());
                return pages.front().StoreUnsafe(str);
            }

            // No capacity --> allocate new page
            return pages.emplace_front().StoreUnsafe(str);
        }

    private:
        std::forward_list<Page> pages{};
    };

} // namespace tagliatelle