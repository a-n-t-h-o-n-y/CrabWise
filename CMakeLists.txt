cmake_minimum_required(VERSION 3.14)
project(Crabwise LANGUAGES CXX)

add_subdirectory(external/TermOx EXCLUDE_FROM_ALL)

add_subdirectory(external/ntwk EXCLUDE_FROM_ALL)

set(SIMDJSON_BUILD_STATIC ON)
add_subdirectory(external/simdjson EXCLUDE_FROM_ALL)

add_subdirectory(src)
