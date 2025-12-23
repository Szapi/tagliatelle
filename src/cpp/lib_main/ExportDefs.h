#pragma once

#ifdef _WIN32
    #ifdef TAGLIATELLE_EXPORTS
        #define TAGLIATELLE_API __declspec(dllexport)
    #else
        #define TAGLIATELLE_API __declspec(dllimport)
    #endif
#else
    #define TAGLIATELLE_API __attribute__((visibility("default")))
#endif
