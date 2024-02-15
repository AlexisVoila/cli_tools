cmake_minimum_required(VERSION 3.16)

project("cli_parser" C CXX)

include(CTest)

option(CLI_PARSER_TESTING "Enable unit tests" ON)

add_library(${PROJECT_NAME} STATIC "")
add_library(cli_tools::parser ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME} 
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/cli_parser.h"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/cli_parser.cpp"
)

# Export include interface
target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
)

# Internal include interface
target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
# TODO: Add tests and install targets if needed.
