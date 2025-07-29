#include <catch2/catch_test_macros.hpp>

#include <cstdint>

#include "WordDictionary.hpp"
#include "StableTextBuffer.hpp"

uint32_t factorial3( uint32_t number ) {
    return number <= 1 ? number : factorial3(number-1) * number;
}

TEST_CASE( "Factorials are computed 3", "[factorial]" ) {
    REQUIRE( factorial3( 1) == 1 );
    REQUIRE( factorial3( 2) == 2 );
    REQUIRE( factorial3( 3) == 6 );
    REQUIRE( factorial3(10) == 3'628'800 );

    using namespace tagliatelle::_detail;

    tagliatelle::StableTextBuffer<1024> tbuf;
    tagliatelle::WordDictionary dict{With<unsigned int>{}, tbuf.GetInterface(), Constant<unsigned int, 512>{}};
    
    REQUIRE( dict.Encode("") == 0u);
    REQUIRE( dict.Encode("a") == 1u);
    REQUIRE( dict.Encode("a") == 1u);
    REQUIRE( dict.Encode("b") == 2u);
    REQUIRE( dict.Encode("a") == 1u);
    REQUIRE( dict.Encode("abc") == 3u);

    auto exp = dict.GetExpander();
    REQUIRE( exp.Encode("") == 0u);
    REQUIRE( exp.Encode("a") == 1u);
    REQUIRE( exp.Encode("a") == 1u);
    REQUIRE( exp.Encode("b") == 2u);
    REQUIRE( exp.Encode("a") == 1u);
    REQUIRE( exp.Encode("abc") == 3u);

    REQUIRE( exp.Encode("bca") == 4u);
    REQUIRE( exp.Encode("cba") == 5u);

    REQUIRE( dict.Merge(std::move(exp)) );
    REQUIRE( dict.Encode("bca") == 4u);
    REQUIRE( dict.Encode("cba") == 5u);
}
