#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "cli_parser.h"

TEST_CASE("Testing cli::param class construction with values") {
    const auto parm{cli::param()
        .set_short_name("-t")
        .set_long_name("--test")
        .set_description("test parameter")
        .set_default_value("127.0.0.1")
        .set_value("192.168.1.1")
        .set_required()
        .set_flag()};

    CHECK(parm.is_parsed() == false);
    CHECK(parm.is_flag());
    CHECK(parm.is_required());
    CHECK(parm.short_name() == "-t");
    CHECK(parm.long_name() == "--test");
    CHECK(parm.description() == "test parameter");
    CHECK(parm.default_value() == "127.0.0.1");
    CHECK(parm.value() == "192.168.1.1");
}

TEST_CASE("Testing cli_parser::parameter class default construction") {
    cli::param parm;

    REQUIRE(parm.is_flag() == false);
    REQUIRE(parm.is_required() == false);
    REQUIRE(parm.short_name() == "");
    REQUIRE(parm.long_name() == "");
    REQUIRE(parm.description() == "");
    REQUIRE(parm.default_value() == "");
    REQUIRE(parm.value() == "");
}

TEST_CASE("Testing cli::param class get_value_as[_str] methods") {
    cli::param parm;

    parm.set_value("123456789");
    CHECK(parm.get_value_as<int>() == 123456789);

    parm.set_value("12345");
    CHECK(parm.get_value_as<std::uint16_t>() == std::uint16_t{12345u});

    parm.set_value("10.1");
    CHECK(parm.get_value_as<float>() == 10.1f);
    CHECK(parm.get_value_as<double>() == double{10.1});

    parm.set_value("a");
    CHECK(parm.get_value_as<char>() == 'a');

    parm.set_value("abcd");
    CHECK(parm.get_value_as_str() == std::string{"abcd"});
}
