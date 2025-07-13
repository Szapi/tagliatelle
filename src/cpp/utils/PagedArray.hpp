#pragma once

// TODO correct includes
#include <algorithm> // std::copy_n, std::ranges::for_each
#include <array>
#include <forward_list>
#include <string_view>
#include <vector>

#include "Utils.hpp"

namespace tagliatelle
{

    template<class T, std::size_t PageSz>
    class PagedArray
    {
        static_assert(std::is_same_v<T, std::remove_cvref_t<T>>, "Invalid PagedArray type");

#ifdef ENFORCE_PAGED_ARRAY_PAGE_SIZE_CONSTRAINT
        static_assert(_detail::IsPowerOfTwo(PageSz), "PagedArray page size should be a power of 2");
#endif

        using Ref = T&;
        using ConstRef = const T&;

        class Page
        {
        public:
            Page()
            {
                data.reserve(PageSz);
            }

            bool Empty() const
            {
                return data.empty();
            }

            std::size_t Size() const
            {
                return data.size();
            }
            
            bool CanStore(const std::size_t n)
            {
                const auto remainingCapacity = PageSz - Size();
                return n <= remainingCapacity;
            }

            // Destroy all stored objects, but keep the allocated memory
            void Recycle()
            {
                data.clear();
            }

            Ref At(const auto idx)
            {
                ASSERT(
                    _detail::ValidIndexZB(idx, data),
                    _F("Item index {} is out of range, data size: {}",
                        idx,
                        data.size())
                );
                return data.at(static_cast<std::size_t>(idx));
            }

            ConstRef At(const auto idx) const
            {
                return const_cast<Page*>(this)->At(idx);
            }

            template<class... Args>
            Ref EmplaceUnsafe(Args&&... args)
            {
                return data.emplace_back(std::forward<Args>(args)...);
            }

        private:
            std::vector<T> data;
        };

        Page& NthPage(const std::size_t idx)
        {
            ASSERT(
                _detail::ValidIndexZB(idx, pages),
                _F("Page index {} is out of range, number of pages: {}",
                    idx,
                    pages.size())
            );
            return pages.at(idx);
        }

        const Page& NthPage(const std::size_t idx) const
        {
            return const_cast<PagedArray*>(this)->NthPage(idx);
        }

    public:
        PagedArray() = default;

        MOVE_ONLY(PagedArray);

        [[nodiscard]] Ref At(const auto idxUntyped)
        {
            const auto idx     = static_cast<std::size_t>(idxUntyped);
            const auto pageIdx = idx / PageSz;
            const auto itemIdx = idx % PageSz;
            return NthPage(pageIdx).At(itemIdx);
        }
        
        [[nodiscard]] ConstRef At(const auto idxUntyped) const
        {
            return const_cast<PagedArray*>(this)->At(idxUntyped);
        }

        void Clear()
        {
            pages.clear();
            currentPage = 0;
        }

        void Recycle()
        {
            for (auto& p : pages)
                p.Recycle();
            currentPage = 0;
        }

        void Prune()
        {
            if (pages.empty())
                return;

            if (0 == currentPage && pages.front().Empty())
            {
                pages.clear();
                return;
            }

            const auto usedPages = currentPage + 1;
            while(pages.size() > usedPages)
                pages.pop_back();
        }

        template<class... Args>
        Ref Emplace(Args&&... args)
        {
            if (pages.empty()) [[unlikely]]
            {
                pages.emplace_back();
                currentPage = 0;
            }

            if (!NthPage(currentPage).CanFit(1)) [[unlikely]]
            {
                pages.emplace_back();
                ++currentPage;
            }

            return NthPage(currentPage).EmplaceUnsafe(std::forward<Args>(args)...);
        }

        [[nodiscard]] std::size_t Size() const
        {
            if (pages.empty())
                return 0;

            const auto fullPages = currentPage;
            return (fullPages * PageSz) + NthPage(currentPage).Size();
        }

    private:
        std::vector<Page> pages;
        std::size_t       currentPage = 0; // Zero-based index of the last non-empty page
    };

} // namespace tagliatelle
