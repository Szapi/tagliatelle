#pragma once

#include "Permit.hpp"

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


#define SINGLETON(Class)            \
    public: IMMOVABLE(Class);       \
    inline static Class& Instance() { static Class inst{}; return inst; } \
    private: Class() = default
