#pragma once

#ifdef ENABLE_DEBUG_MACROS

    #include <stdexcept>

    #include "FormatUtils.hpp"

    #define ASSERT(expr,msg) if (!expr) [[unlikely]] throw std::runtime_error(std::string{msg})

#else

    #define ASSERT(expr,msg)

#endif
