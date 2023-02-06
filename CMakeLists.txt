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
endif()

option(CLI_TOOLS_TESTING "Enable unit tests" ON)

include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
    doctest
    PREFIX ${CMAKE_BINARY_DIR}/doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)

# Expose required variable (DOCTEST_INCLUDE_DIR) to parent scope
ExternalProject_Get_Property(doctest source_dir)
set(DOCTEST_INCLUDE_DIR ${source_dir}/doctest CACHE INTERNAL "Path to include folder for doctest")

# Include sub-projects.
add_subdirectory (cli_parser)
