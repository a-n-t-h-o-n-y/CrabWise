cmake_minimum_required(VERSION 3.14)

add_library(markets
    network/websocket.cpp
    network/https_socket.cpp
    coinbase.cpp

    # market/bitstamp.cpp
    # market/poloniex.cpp
    # market/binance.cpp
    # market/bittrex.cpp
)

find_package(Boost 1.66 REQUIRED COMPONENTS date_time system program_options)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
target_link_libraries(markets PUBLIC
    ${Boost_LIBRARIES}
    Threads::Threads
    OpenSSL::Crypto
    OpenSSL::SSL
)

target_include_directories(markets PUBLIC ${Boost_INCLUDE_DIR})
target_compile_features(markets PRIVATE cxx_std_17)
target_compile_options(markets PRIVATE -Wall -Wextra)
