# Define source files
set(TAGLIATELLE_SOURCES
    Engine.cpp
    StaticAssertions.cpp
)

# Create shared library (DLL)
add_library(tagliatelle SHARED
    ${TAGLIATELLE_SOURCES}
    tagliatelle_api.cpp
)

# Create static library (for internal testing)
add_library(tagliatelle_static STATIC
    ${TAGLIATELLE_SOURCES}
)

# Set common library properties
foreach(target tagliatelle tagliatelle_static)
    set_target_properties(${target} PROPERTIES
        VERSION ${PROJECT_VERSION}
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
    
    target_include_directories(${target} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    )
endforeach()

# Set shared library specific properties
set_target_properties(tagliatelle PROPERTIES
    SOVERSION 1
)

# Set shared library specific compile definitions
target_compile_definitions(tagliatelle PRIVATE TAGLIATELLE_EXPORTS)

