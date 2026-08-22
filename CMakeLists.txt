cmake_minimum_required(VERSION 3.31)

project(
        skoll
        VERSION 0.1.0
        LANGUAGES CXX
)

find_package(
        nlohmann_json
        CONFIG
        REQUIRED
)

find_package(
        ixwebsocket
        CONFIG
        REQUIRED
)

add_executable(
        skoll
        main.cpp
)

add_library(
        skoll_core
        src/feed/message.cpp
        src/feed/client.cpp
)

set_target_properties(skoll PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

target_include_directories(
        skoll_core
        PUBLIC
        include
)

target_compile_features(
        skoll_core
        PUBLIC
        cxx_std_20
)

# skoll_core requires nlohmann_json
target_link_libraries(
        skoll_core
        PRIVATE
        nlohmann_json::nlohmann_json
        PUBLIC
        ixwebsocket::ixwebsocket
)

# the skoll executable requires skoll_core
target_link_libraries(
        skoll
        PRIVATE
        skoll_core
)