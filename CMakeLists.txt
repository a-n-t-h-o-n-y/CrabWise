cmake_minimum_required(VERSION 3.14)
project(Crabwise LANGUAGES CXX)

add_subdirectory(external/TermOx)

add_subdirectory(external/ntwk)

set(SIMDJSON_BUILD_STATIC ON)
add_subdirectory(external/simdjson)

add_subdirectory(src)
