cmake_minimum_required(VERSION 3.31)
project(skoll VERSION 0.1.0 LANGUAGES CXX)

add_executable(skoll main.cpp)

set_target_properties(skoll PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)