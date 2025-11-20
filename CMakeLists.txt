cmake_minimum_required(VERSION 3.14)
project(StingerUtils VERSION 0.1.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(STINGER_UTILS_BUILD_TESTS "Build tests" ON)
option(STINGER_UTILS_BUILD_EXAMPLES "Build examples" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Find libmosquitto
find_library(MOSQUITTO_LIBRARY mosquitto REQUIRED)
find_path(MOSQUITTO_INCLUDE_DIR mosquitto.h REQUIRED)

# Library sources
set(STINGER_UTILS_SOURCES
    src/connection.cpp
)

# Library headers
set(STINGER_UTILS_HEADERS
    include/stinger/utils/iconnection.hpp
    include/stinger/utils/connection.hpp
)

# Create library
add_library(stinger_utils ${STINGER_UTILS_SOURCES} ${STINGER_UTILS_HEADERS})
add_library(Stinger::Utils ALIAS stinger_utils)

# Include directories
target_include_directories(stinger_utils
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${MOSQUITTO_INCLUDE_DIR}
)

# Link libmosquitto
target_link_libraries(stinger_utils PUBLIC ${MOSQUITTO_LIBRARY})

# Set compile features
target_compile_features(stinger_utils PUBLIC cxx_std_17)

# Add tests
if(STINGER_UTILS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Add examples
if(STINGER_UTILS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation rules
include(GNUInstallDirs)

install(TARGETS stinger_utils
    EXPORT StingerUtilsTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT StingerUtilsTargets
    FILE StingerUtilsTargets.cmake
    NAMESPACE Stinger::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/StingerUtils
)

# Create config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/StingerUtilsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/StingerUtilsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/StingerUtils
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/StingerUtilsConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/StingerUtilsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/StingerUtilsConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/StingerUtils
)
