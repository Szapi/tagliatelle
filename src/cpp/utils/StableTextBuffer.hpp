#pragma once

#include <algorithm> // std::copy_n, std::ranges::for_each
#include <array>
#include <forward_list>
#include <string_view>

#include "StableTextStorageInterfaceConcept.hpp"
#include "Utils.hpp"

namespace tagliatelle
{

    // Text buffer that can grow indefinitely without invalidating
    // existing views to this buffer.
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

            [[nodiscard]] std::string_view StoreUnsafe(const std::string_view str)
            {
                ASSERT(
                    CanFit(str),
                    _F("StoreUnsafe: cannot fit string of length {}, remaining space is {}, page size is {}",
                        str.size(), PageSz - occupied, PageSz)
                );
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

            Page() = default;
            IMMOVABLE(Page);

        private:
            std::array<char, PageSz> data;
            std::size_t occupied = 0;
        };

    public:
        // Basically a delegate
        class StorageInterface
        {
        public:
            StorageInterface(StableTextBuffer& bufIn) : buf {&bufIn} {}
            COPYABLE(StorageInterface);

            [[nodiscard]] std::string_view Store(std::string_view str)
            {
                return buf->Store(str);
            }

        private:
            StableTextBuffer* buf;
        };

        static_assert(concepts::StableTextStorageInterface<StorageInterface>);

        [[nodiscard]] StorageInterface GetInterface() { return {*this}; }

    public:
        StableTextBuffer() = default;

        MOVE_ONLY(StableTextBuffer);

        // Deallocate all pages, unlike STL's clear()
        void Clear()
        {
            pages.clear();
            recycledPages.clear();
        }

        // Reuse allocated pages, equivalent to STL's clear()
        // Recycled pages are reused in the order of allocation
        void Recycle()
        {
            recycledPages.clear();
            auto RecycleOne = [this, head = true](Page& p) mutable
                {
                    p.Recycle();
                    if (!head)
                        recycledPages.push_back(&p);
                    head = false;
                };
            std::ranges::for_each(pages, RecycleOne);
        }

        // Deallocate unused pages
        void Prune()
        {
            recycledPages.clear();
            while (!pages.empty() && pages.front().Empty())
                pages.pop_front();
        }

        // Copies the given string to the buffer and returns a view of it
        [[nodiscard]] std::string_view Store(const std::string_view str)
        {
            ASSERT(str.size() <= PageSz, _F("StableTextBuffer: cannot fit string of length {}, page size is only {}", str.size(), PageSz));

            if (str.empty()) [[unlikely]]
                return str;

            if (pages.empty()) [[unlikely]]
                return pages.emplace_front().StoreUnsafe(str);

        TryRecycled:
            if (!recycledPages.empty())
            {
                if (recycledPages.back()->CanFit(str)) [[likely]]
                    return recycledPages.back()->StoreUnsafe(str);

                recycledPages.pop_back();
                goto TryRecycled;
            }

            if (!pages.front().CanFit(str)) [[unlikely]]
                pages.emplace_front();

            return pages.front().StoreUnsafe(str);
        }

    private:
        std::forward_list<Page> pages;
        std::vector<Page*>      recycledPages;
    };

} // namespace tagliatelle
