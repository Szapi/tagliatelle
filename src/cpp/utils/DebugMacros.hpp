#pragma once

#ifdef ENABLE_DEBUG_MACROS

    #include <format>
    #include <stdexcept>

    #define _F std::format

    #define ASSERT(expr,msg) if (!expr) [[unlikely]] throw std::runtime_error(msg)

#else

    #define ASSERT(expr,msg)

#endif
