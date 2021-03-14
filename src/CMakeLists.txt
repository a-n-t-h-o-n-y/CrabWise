cmake_minimum_required(VERSION 3.14)

add_subdirectory(markets EXCLUDE_FROM_ALL)

add_executable(crabwise
    log.cpp
    symbol_id_json.cpp
    crabwise.main.cpp
)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.66 REQUIRED COMPONENTS filesystem)

target_link_libraries(crabwise
    PRIVATE
        TermOx
        markets
        ntwk
        simdjson
        Boost::filesystem
)

include(GNUInstallDirs)
install(TARGETS crabwise
        RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        CONFIGURATIONS Release
)
