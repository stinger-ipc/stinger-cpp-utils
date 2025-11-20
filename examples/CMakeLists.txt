cmake_minimum_required(VERSION 3.14)

add_executable(example_usage example_usage.cpp)

target_link_libraries(example_usage
    PRIVATE
        stinger_utils
)

target_include_directories(example_usage
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)
