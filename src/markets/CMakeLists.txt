cmake_minimum_required(VERSION 3.14)

add_library(markets
    coinbase.cpp
    finnhub.cpp
)

target_link_libraries(markets PUBLIC ntwk)
target_compile_features(markets PRIVATE cxx_std_17)
target_compile_options(markets PRIVATE -Wall -Wextra)

# Generate Symbol IDs Tables Application
add_executable(generate_ids generate_ids.main.cpp)

find_package(Threads REQUIRED)
target_link_libraries(generate_ids
    PRIVATE
        ntwk
        ${CMAKE_THREAD_LIBS_INIT}
)

target_compile_features(generate_ids PRIVATE cxx_std_17)
target_compile_options(generate_ids PRIVATE -Wall -Wextra)
