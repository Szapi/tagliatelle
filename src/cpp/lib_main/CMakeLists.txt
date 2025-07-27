# Create the dynamic library
add_library(tagliatelle SHARED
    tagliatelle_api.cpp
    Tagliatelle.cpp
)

# Set library properties
set_target_properties(tagliatelle PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Include directories
target_include_directories(tagliatelle PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Export symbols for Windows DLL
if(WIN32)
    target_compile_definitions(tagliatelle PRIVATE TAGLIATELLE_EXPORTS)
endif()
