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


set(CLANG_P2996_INSTALL_DIR N:\\Dev\\clang_reflection_install_2)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/lib_main/asdf/asdf.exe
    MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/lib_main/asdf.cpp
    COMMAND ${CLANG_P2996_INSTALL_DIR}\\bin\\clang++.exe ${CMAKE_SOURCE_DIR}/lib_main/asdf.cpp -std=c++26 -freflection-latest -I${CLANG_P2996_INSTALL_DIR}\\include\\c++\\v1 -L${CLANG_P2996_INSTALL_DIR}\\lib -o ${CMAKE_BINARY_DIR}/lib_main/asdf/asdf.exe)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/lib_main/incl/valami.hpp
    MAIN_DEPENDENCY ${CMAKE_BINARY_DIR}/lib_main/asdf/asdf.exe
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/lib_main/incl
    COMMAND ${CMAKE_BINARY_DIR}/lib_main/asdf/asdf.exe > ${CMAKE_BINARY_DIR}/lib_main/incl/valami.hpp)

add_custom_target(GeneratedHeaders DEPENDS ${CMAKE_BINARY_DIR}/lib_main/incl/valami.hpp)

add_dependencies(tagliatelle GeneratedHeaders)