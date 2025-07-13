add_executable(tests StableTextBufferTest.cpp PagedArrayTest.cpp WordDictionaryTest.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

target_include_directories(tests PUBLIC ../lib_main)

include(CTest)
include(Catch)
catch_discover_tests(tests)