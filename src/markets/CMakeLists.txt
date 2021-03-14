cmake_minimum_required(VERSION 3.14)

add_library(markets
    coinbase.cpp
    finnhub.cpp
)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.66 REQUIRED COMPONENTS filesystem)

target_link_libraries(markets PRIVATE ntwk simdjson Boost::filesystem)
target_compile_features(markets PRIVATE cxx_std_17)
target_compile_options(markets PRIVATE -Wall -Wextra)
