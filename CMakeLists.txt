cmake_minimum_required(VERSION 3.14)
project(StingerUtils VERSION 0.5.4 LANGUAGES CXX)

# Include standard installation directories
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Options
option(STINGER_UTILS_BUILD_TESTS "Build tests" ON)
option(STINGER_UTILS_BUILD_EXAMPLES "Build examples" ON)
option(STINGER_UTILS_BUILD_MOCK "Build mock connection for testing" OFF)

# Find libmosquitto using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(mosquitto REQUIRED libmosquitto)

# Library sources
set(STINGER_UTILS_SOURCES
    src/conversions.cpp
    src/format.cpp
    src/hash.cpp
    src/mqttbrokerconnection.cpp
    src/mqttmessage.cpp
    src/return_codes.cpp
    $<$<BOOL:${STINGER_UTILS_BUILD_MOCK}>:src/mockconnection.cpp>
)

# Library headers
set(STINGER_UTILS_HEADERS
    include/stinger/utils/conversions.hpp
    include/stinger/utils/format.hpp
    include/stinger/utils/hash.hpp
    include/stinger/utils/iconnection.hpp
    include/stinger/mqtt/brokerconnection.hpp
    include/stinger/mqtt/message.hpp
    include/stinger/mqtt/properties.hpp
    include/stinger/utils/uuid.hpp
    include/stinger/error/return_codes.hpp
    $<$<BOOL:${STINGER_UTILS_BUILD_MOCK}>:include/stinger/utils/mockconnection.hpp>
)

if(STINGER_UTILS_BUILD_MOCK)
    message(STATUS "Building with MockConnection support")
endif()

# Create library
add_library(stinger_utils ${STINGER_UTILS_SOURCES} ${STINGER_UTILS_HEADERS})
add_library(Stinger::Utils ALIAS stinger_utils)

# Include directories
target_include_directories(stinger_utils
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${mosquitto_INCLUDE_DIRS}
)

# Link libmosquitto
target_link_libraries(stinger_utils PUBLIC ${mosquitto_LIBRARIES})

# Set target properties
set_target_properties(stinger_utils
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        POSITION_INDEPENDENT_CODE ON
)

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
