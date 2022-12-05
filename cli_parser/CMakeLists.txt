cmake_minimum_required(VERSION 3.16)

project("cli_parser" CXX)

add_library(${PROJECT_NAME} STATIC "")

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_sources(${PROJECT_NAME} PUBLIC
    "cli_parser.cpp" 
    "cli_parser.h"
)

if(MSVC)
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


# TODO: Add tests and install targets if needed.
