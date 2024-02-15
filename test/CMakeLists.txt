cmake_minimum_required(VERSION 3.16)

project("cli_parser_test" CXX)

if (MSVC)
    set(CMAKE_VS_JUST_MY_CODE_DEBUGGING "$<$<CONFIG:Debug>:ON>")
endif()

option(CLI_PARSER_TESTING "Enable unit tests" ON)


if (CLI_PARSER_TESTING)
    add_executable(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/cli_parser_test.cpp")

    FetchContent_Declare(doctest
        URL https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h
        DOWNLOAD_NO_EXTRACT 1
    )
    FetchContent_MakeAvailable(doctest)

    message(STATUS "${PROJECT_NAME} tests enabled")

    target_include_directories(${PROJECT_NAME} PRIVATE ${doctest_SOURCE_DIR})

    target_link_libraries(${PROJECT_NAME} PRIVATE cli_tools::parser)

    add_custom_target(check ALL COMMAND ${PROJECT_NAME})

endif()

# TODO: Add tests and install targets if needed.
