cmake_minimum_required(VERSION 3.14)

add_executable(crabwise
    crabwise.main.cpp
)

target_link_libraries(crabwise PRIVATE TermOx)
