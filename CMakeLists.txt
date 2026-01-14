cmake_minimum_required(VERSION 3.30)
project(wots VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

find_package(spdlog REQUIRED)
find_package(GTest REQUIRED)

add_library(wots STATIC)
target_sources(wots
    PRIVATE
        wots/wots.cpp
)
target_include_directories(wots
    PUBLIC
        ${CMAKE_SOURCE_DIR}
)
target_link_libraries(wots
    PUBLIC
        spdlog::spdlog
)

add_executable(wots-cli)
target_sources(wots-cli
    PRIVATE
        wots-cli/main.cpp
)
target_link_libraries(wots-cli
    PRIVATE
        wots
)
set_target_properties(wots-cli
    PROPERTIES
        OUTPUT_NAME wots
)
