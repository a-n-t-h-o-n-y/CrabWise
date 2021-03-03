cmake_minimum_required(VERSION 3.14)

# Networking and Markets
add_library(network
    network/websocket.cpp
    network/https_socket.cpp
)

find_package(Boost 1.66 REQUIRED COMPONENTS date_time system program_options)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
target_link_libraries(network PUBLIC
    ${Boost_LIBRARIES}
    Threads::Threads
    OpenSSL::Crypto
    OpenSSL::SSL
)
target_include_directories(network PUBLIC ${Boost_INCLUDE_DIR})
target_compile_features(network PRIVATE cxx_std_17)
target_compile_options(network PRIVATE -Wall -Wextra)

add_library(markets
    coinbase.cpp
    finnhub.cpp
)

target_link_libraries(markets PUBLIC network)
target_compile_features(markets PRIVATE cxx_std_17)
target_compile_options(markets PRIVATE -Wall -Wextra)

# Generate Symbol IDs Tables Application
add_executable(generate_ids generate_ids.main.cpp)
target_link_libraries(generate_ids PRIVATE network)
target_compile_features(generate_ids PRIVATE cxx_std_17)
target_compile_options(generate_ids PRIVATE -Wall -Wextra)
