# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.16)
project ("cli_tools" CXX)

get_directory_property(IS_SUBPROJECT PARENT_DIRECTORY)

if (IS_SUBPROJECT AND NOT DEFINED CLI_TOOLS_TESTING)
    message(STATUS "${PROJECT_NAME} unit tests are disabled subproject mode")
    set(CLI_TOOLS_TESTING OFF)
else()
    option(CLI_PARSER_TESTING "Enable unit tests" ON)
    include(FetchContent)
    find_package(Git REQUIRED)
endif()

# Include sub-projects.
add_subdirectory (cli_parser)
