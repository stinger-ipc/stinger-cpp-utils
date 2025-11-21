cmake_minimum_required(VERSION 3.14)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Test executable
add_executable(stinger_utils_tests
    test_mqttmessage.cpp
)

# Add mock connection tests if enabled
if(STINGER_UTILS_BUILD_MOCK)
    target_sources(stinger_utils_tests PRIVATE test_mockconnection.cpp)
endif()

target_link_libraries(stinger_utils_tests
    PRIVATE
        stinger_utils
        GTest::gtest_main
)

target_include_directories(stinger_utils_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

include(GoogleTest)
gtest_discover_tests(stinger_utils_tests)
