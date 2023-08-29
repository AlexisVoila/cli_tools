#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "cli_parser.h"

TEST_SUITE("Testing cli::Param" * doctest::description("Class cli::Param tests")) {
    TEST_CASE("Testing cli::Param class construction with values") {
        const auto parm{cli::Param()
            .short_name("-t")
            .long_name("--test")
            .description("test parameter")
            .default_value("127.0.0.1")
            .value("192.168.1.1")
            .required()
            .flag()};

        CHECK(parm.is_parsed() == false);
        CHECK(parm.is_flag());
        CHECK(parm.is_required());
        CHECK(parm.short_name() == "t");
        CHECK(parm.long_name() == "test");
        CHECK(parm.description() == "test parameter");
        CHECK(parm.default_value() == "127.0.0.1");
        CHECK(parm.value() == "192.168.1.1");
    }

    TEST_CASE("Testing cli_parser::parameter class default construction") {
        cli::Param parm;

        REQUIRE(parm.is_flag() == false);
        REQUIRE(parm.is_required() == false);
        REQUIRE(parm.short_name() == "");
        REQUIRE(parm.long_name() == "");
        REQUIRE(parm.description() == "");
        REQUIRE(parm.default_value() == "");
        REQUIRE(parm.value() == "");
    }

    TEST_CASE("Testing cli::Param class get_value_as[_str] methods") {
        cli::Param parm;

        parm.value("123456789");
        CHECK(parm.get_value_as<int>() == 123456789);

        parm.value("12345");
        CHECK(parm.get_value_as<std::uint16_t>() == std::uint16_t{12345u});

        parm.value("10.1");
        CHECK(parm.get_value_as<float>() == doctest::Approx(10.1f));
        CHECK(parm.get_value_as<double>() == doctest::Approx(10.1));

        parm.value("a");
        CHECK(parm.get_value_as<char>() == 'a');

        parm.value("abcd");
        CHECK(parm.get_value_as_str() == std::string{"abcd"});
    }
}

TEST_SUITE("Testing cli::ParamParser" * doctest::description("Class cli::ParamParser tests")) {
    TEST_CASE("Testing the creation and preparation of the cli::ParamParser class for parsing") {
        cli::ParamParser cli_parser;

        REQUIRE(cli_parser.parameters_count() == 0);

        SUBCASE("Checking the number of parameters added") {
            cli_parser
                .add_parameter(cli::Param().short_name("-h").long_name("--help").flag().description("show help message"))
                .add_parameter(cli::Param().short_name("-p").long_name("--port").required().default_value("8080").description("listen port"))
                .add_parameter(cli::Param().short_name("-a").long_name("--ip-address").required().description("ip address"));

            REQUIRE(cli_parser.parameters_count() == 3);
        }

        SUBCASE("Checking the number of parameters added with duplication") {
            cli_parser.add_parameter(cli::Param().short_name("-h"));
            cli_parser.add_parameter(cli::Param().short_name("-h"));

            REQUIRE(cli_parser.parameters_count() == 1);
        }
        SUBCASE("Checking the number of parameters added with partial duplication (short keys)") {
            cli_parser.add_parameter(cli::Param().short_name("-h").long_name("--help"));
            cli_parser.add_parameter(cli::Param().short_name("-h").long_name("--hhhh"));

            REQUIRE(cli_parser.parameters_count() == 1);
            REQUIRE(cli_parser.all_params()[0]->long_name() == "hhhh");
        }
        SUBCASE("Checking the number of parameters added with partial duplication (long keys)") {
            cli_parser.add_parameter(cli::Param().short_name("-h").long_name("--help"));
            cli_parser.add_parameter(cli::Param().short_name("-a").long_name("--help"));

            REQUIRE(cli_parser.parameters_count() == 1);
            REQUIRE(cli_parser.all_params()[0]->short_name() == "a");
        }
    }

    TEST_CASE("Testing cli::ParamParser reset() method") {
        cli::ParamParser cli_parser;

        cli_parser.add_parameter(cli::Param().short_name("-h"));
        cli_parser.reset();

        CHECK(cli_parser.parameters_count() == 0);
    }

    TEST_CASE("Testing cli::ParamParser parse result") {
        constexpr int argc = 5;
        char* argv[argc] = {"program.exe", "--help", "--port=8080", "-a", "127.0.0.1"};

        cli::ParamParser cli_parser;
        cli_parser
            .add_parameter(cli::Param().short_name("-h").long_name("--help").flag().description("show help message"))
            .add_parameter(cli::Param().short_name("-p").long_name("--port").required().default_value("8080").description("listen port"))
            .add_parameter(cli::Param().short_name("-a").long_name("--ip-address").required().description("ip address"));

        cli_parser.parse(argc, argv);

        CHECK(cli_parser.all_params().size() == 3);
        CHECK(cli_parser.arg("h").is_parsed());
        CHECK(cli_parser.arg("help").is_parsed());
        CHECK(cli_parser.arg("help").is_flag());

        CHECK(cli_parser.arg("p").is_parsed());
        CHECK(cli_parser.arg("port").is_parsed());
        CHECK(cli_parser.arg("p").get_value_as<uint16_t>() == 8080);
        CHECK(!cli_parser.arg("p").is_flag());
        CHECK(cli_parser.arg("a").is_parsed());
        CHECK(cli_parser.arg("ip-address").is_parsed());
        CHECK(cli_parser.arg("a").get_value_as<std::string>() == "127.0.0.1");
        CHECK(!cli_parser.arg("a").is_flag());
    }

    TEST_CASE("Testing cli::ParamParser parse result (default values)") {
        std::vector<std::string> args{"program.exe", "--port=8080"};

        cli::ParamParser cli_parser;
        cli_parser
            .add_parameter(cli::Param().short_name("-h").long_name("--help").flag().description("show help message"))
            .add_parameter(cli::Param().short_name("-p").long_name("--port").required().default_value("8080").description("listen port"))
            .add_parameter(cli::Param().short_name("-a").long_name("--ip-address").default_value("127.0.0.1").description("ip address"));

        cli_parser.parse(args);

        CHECK(cli_parser.all_params().size() == 3);
        CHECK(cli_parser.arg("p").is_parsed());
        CHECK(cli_parser.arg("port").is_parsed());
        CHECK(cli_parser.arg("p").get_value_as<uint16_t>() == 8080);
        CHECK(!cli_parser.arg("a").is_parsed());
        CHECK(cli_parser.arg("ip-address").default_value() == "127.0.0.1");
        CHECK(cli_parser.arg("a").get_value_as<std::string>() == "127.0.0.1");
    }
}

