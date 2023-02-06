cmake_minimum_required(VERSION 3.16)

project("cli_parser" CXX)

option(CLI_PARSER_TESTING "Enable unit tests" ON)

add_library(${PROJECT_NAME} STATIC "")
add_library(cli_tools::parser ALIAS ${PROJECT_NAME})

if (CLI_PARSER_TESTING)
    message(STATUS "${PROJECT_NAME} tests enabled")
    add_executable(cli_parser_tests "cli_parser_test.cpp")
    target_include_directories(cli_parser_tests PRIVATE ${DOCTEST_INCLUDE_DIR})
    target_compile_features(cli_parser_tests PRIVATE cxx_std_17)
    target_link_libraries(cli_parser_tests PRIVATE ${PROJECT_NAME})
    add_custom_target(check ALL COMMAND cli_parser_tests)
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_sources(${PROJECT_NAME} PUBLIC
    "cli_parser.cpp" 
    "cli_parser.h"
)

if ((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Werror

        -Wall
        -Wextra
        -Wpedantic

        -Wcast-align
        -Wcast-qual
        -Wconversion
        -Wctor-dtor-privacy
        -Wenum-compare
        -Wfloat-equal
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wredundant-decls
        -Wsign-conversion
        -Wsign-promo
    )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)

    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        "_WIN32_WINNT=0x0A00"
        "$<$<CONFIG:Release>:/NDEBUG>"
    )

    set_property(TARGET ${PROJECT_NAME} PROPERTY 
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )

    target_link_options(${PROJECT_NAME} PRIVATE 
        "$<$<CONFIG:Release>:/INCREMENTAL:NO>"
		"$<$<CONFIG:Release>:/OPT:REF>"
		"$<$<CONFIG:Release>:/OPT:ICF>"
	)
endif()

if (NOT CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

# TODO: Add tests and install targets if needed.
