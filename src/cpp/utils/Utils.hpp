#pragma once

#include "DebugMacros.hpp"


#define IMMOVABLE(Class)            \
    Class(const Class&) = delete;   \
    Class& operator=(const Class&) = delete


#define COPYABLE(Class)             \
    Class(const Class&) = default;  \
    Class& operator=(const Class&) = default


#define MOVE_ONLY(Class)            \
    IMMOVABLE(Class);               \
    Class(Class&&) = default;       \
    Class& operator=(Class&&) = default
