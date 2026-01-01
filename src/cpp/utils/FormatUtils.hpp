#pragma once

#include <array>
#include <format>

#define _F std::format

#define _SS(bufSz, ...) \
    [&, _my_buf = std::array<char, bufSz>{}]() mutable -> std::string_view \
    { \
        const auto res = std::format_to_n(_my_buf.begin(), _my_buf.size(), __VA_ARGS__); \
        return { &_my_buf.front(), (res.size <= _my_buf.size()) ? res.size : _my_buf.size() }; \
    }()

#define _S(...) _SS(128u, __VA_ARGS__)
