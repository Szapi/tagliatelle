#pragma once

#include <algorithm>
#include <chrono>
#include <iterator>
#include <ranges>
#include <syncstream>

#include "LoggingEndpoint.hpp"

namespace tagliatelle::logging
{

    inline constexpr std::string_view SpellOutLevel(const Level level)
    {
        using namespace std::string_view_literals;
        switch (level)
        {
            case Level::Debug:   return "Debug"sv;
            case Level::Info:    return "Info"sv;
            case Level::Warning: return "Warning"sv;
            case Level::Error:   return "Error"sv;
            case Level::Fatal:   return "Fatal"sv;
            default:             return "INVALID_LEVEL"sv;
        }
    }


    inline constexpr std::array ValidLevels {
        Level::Debug,
        Level::Info,
        Level::Warning,
        Level::Error,
        Level::Fatal
    };


    inline constexpr std::size_t MaxLevelLength =
        std::ranges::max(ValidLevels | std::views::transform(SpellOutLevel) | std::views::transform([](auto str) { return str.length(); }));


    std::string_view MakeStandardLogEntryHeader(const Level level, auto& buf)
    {
        const auto levelStr = SpellOutLevel(level);

        std::array<char, MaxLevelLength> paddedLevelBuf;
        paddedLevelBuf.fill(' ');
        std::strncpy(&paddedLevelBuf.front(), levelStr.data(), levelStr.length());
        const std::string_view paddedLevel{ &paddedLevelBuf.front(), paddedLevelBuf.size() };

        const auto time = std::chrono::system_clock::now();

        const auto res = std::format_to_n(buf.begin(), buf.size(), "{0:%F %T} | {1:s} | ", time, paddedLevel);
        return std::string_view{ &buf.front(), (buf.size() < res.size) ? buf.size() : res.size };
    }


    template<class S, class Dst>
    concept StreamableTo = requires(Dst dst, const S& s)
    {
        dst << s;
    };

    template<class S, class...Dst>
    concept StreamableToAll = (StreamableTo<S, Dst> && ...);

    auto CreateSynchronizedNode(std::derived_from<std::ostream> auto& ...streams)
    {
        return [&](StreamableToAll<decltype(streams)...> auto&&... msgParts)
        {
            auto streamTo = [&msgParts...](auto& ostream)
            {
                (std::osyncstream(ostream) << ... << msgParts) << std::flush;
            };
            (streamTo(streams), ...);
        };
    }

} // namespace tagliatelle::logging
