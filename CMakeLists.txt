cmake_minimum_required(VERSION 3.16)
project(logger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
option(BUILD_TEST "Whether to build tests" OFF)

include_directories(include)

add_library(logger STATIC src/logger.cpp)
if (NOT WIN32)
    target_link_libraries(logger pthread)
endif ()

if (BUILD_TEST)
    message(STATUS "Building the test driver")
    add_executable(test test.cpp)
    target_link_libraries(test logger)
endif ()

# Install steps
set_target_properties(logger PROPERTIES PUBLIC_HEADER include/logger.hpp include/logger_undef.hpp)

install(TARGETS logger
        LIBRARY
            DESTINATION lib
        PUBLIC_HEADER
            DESTINATION include
        )
