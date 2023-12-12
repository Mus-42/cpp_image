cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE LIB_SOURCES "src/*.cpp")

project(CppImage LANGUAGES CXX)

add_library(CppImage STATIC ${LIB_SOURCES})
target_include_directories(CppImage PUBLIC "${CMAKE_SOURCE_DIR}/include")
include(cmake/SetupTargerWarnings.cmake)

setup_target_warnings(CppImage)

# TODO: conditional compilation

file(GLOB_RECURSE EXAMPLES "examples/*.cpp")


foreach(EXAMPLE_SOURCE IN LISTS EXAMPLES)
    string(REPLACE ".cpp" "" EXAMPLE ${EXAMPLE_SOURCE})
    string(REPLACE "${CMAKE_SOURCE_DIR}/examples/" "" EXAMPLE ${EXAMPLE})
    
    message(${EXAMPLE})
    
    add_executable(${EXAMPLE} ${EXAMPLE_SOURCE})
    setup_target_warnings(${EXAMPLE})
    target_include_directories(${EXAMPLE} PUBLIC "${CMAKE_SOURCE_DIR}/include")
    target_link_libraries(${EXAMPLE} PUBLIC CppImage)
endforeach()
