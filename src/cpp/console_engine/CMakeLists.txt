# Create the dynamic library
add_executable(console_engine
    Main.cpp
)

target_link_libraries(console_engine PRIVATE tagliatelle_static)

target_include_directories(console_engine PRIVATE ../lib_main)
