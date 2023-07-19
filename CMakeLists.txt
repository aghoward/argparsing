cmake_minimum_required(VERSION 3.12)

project(ArgParsing)

add_library(argparsing INTERFACE)

find_library(LIB_EITHER NAMES either)
find_library(LIB_CDIF NAMES cdif)

target_include_directories(argparsing INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LIB_EITHER}/include
    ${LIB_CDIF}/include
)
target_compile_features(argparsing INTERFACE cxx_std_17)
