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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A simple example function for the dynamic library
 * @param value An integer value
 * @return The doubled value
 */
TAGLIATELLE_API int tagliatelle_double_value(int value);

/**
 * @brief Get library version string
 * @return Version string
 */
TAGLIATELLE_API const char* tagliatelle_get_version(void);

#ifdef __cplusplus
}
#endif
