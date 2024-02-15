cmake_minimum_required(VERSION 3.16)

project ("cli_tools" VERSION 0.1.0 LANGUAGES C CXX)

get_directory_property(IS_SUBPROJECT PARENT_DIRECTORY)

if(NOT "${CMAKE_CXX_STANDARD}")
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED YES)
    set(CMAKE_CXX_EXTENSIONS NO)
endif()

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Include sub-projects.
add_subdirectory (cli_parser)

if (IS_SUBPROJECT AND NOT DEFINED CLI_TOOLS_TESTING)
    message(STATUS "${PROJECT_NAME} unit tests are disabled by default for subprojects")
    set(CLI_TOOLS_TESTING OFF)
    option(CLI_PARSER_TESTING "Enable unit tests" OFF)
else()
    option(CLI_PARSER_TESTING "Enable unit tests" ON)
    include(FetchContent)
    find_package(Git REQUIRED)
    add_subdirectory (test)
endif()

