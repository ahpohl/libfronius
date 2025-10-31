cmake_minimum_required(VERSION 3.15)

# --- Version from git tag ---
execute_process(
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REGEX REPLACE "^v" "" GIT_TAG_STRIPPED "${GIT_TAG}")

project(
    libfronius
    LANGUAGES CXX
    DESCRIPTION "Library to access Fronius inverters and smart meters"
    VERSION ${GIT_TAG_STRIPPED}
)

# Ensure position independent code for shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Optional: show the build type
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- Sources ---
set(MYLIB_SOURCES src/fronius.cpp src/meter.cpp src/inverter.cpp)

# --- Shared library ---
if(BUILD_SHARED_LIBS)
    add_library(fronius_shared SHARED ${MYLIB_SOURCES})
    target_include_directories(
        fronius_shared
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    set_target_properties(
        fronius_shared
        PROPERTIES
            OUTPUT_NAME "fronius"
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
            CXX_STANDARD 23
            EXPORT_NAME shared
    )
    message(STATUS "Building shared library: fronius_shared")
endif()

# --- Static library ---
if(BUILD_STATIC_LIBS)
    add_library(fronius_static STATIC ${MYLIB_SOURCES})
    target_include_directories(
        fronius_static
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    set_target_properties(
        fronius_static
        PROPERTIES
            OUTPUT_NAME "fronius"
            CXX_STANDARD 23
            EXPORT_NAME static
    )
    message(STATUS "Building static library: fronius_static")
endif()

# --- Install library and headers ---
install(DIRECTORY include/ DESTINATION include/fronius)
if(BUILD_SHARED_LIBS)
    install(TARGETS fronius_shared
        EXPORT froniusTargets
        DESTINATION lib
    )
endif()
if(BUILD_STATIC_LIBS)
    install(TARGETS fronius_static
        EXPORT froniusTargets
        DESTINATION lib
    )
endif()

# --- CMake Package Config ---
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/froniusConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/froniusConfig.cmake"
    INSTALL_DESTINATION lib/cmake/fronius
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/froniusConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/froniusConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/froniusConfigVersion.cmake"
    DESTINATION lib/cmake/fronius
)

install(
    EXPORT froniusTargets
    NAMESPACE fronius::
    DESTINATION lib/cmake/fronius
)

# --- CPack ---
include(InstallRequiredSystemLibraries)
set(CPACK_GENERATOR "TGZ;DEB")
set(CPACK_PACKAGE_NAME "libfronius")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Alexander Pohl <alex@ahpohl.com>")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
  set(DEB_ARCH "arm64")
else()
  set(DEB_ARCH "amd64")
endif()
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${DEB_ARCH}")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_SYSTEM_NAME "")
set(CPACK_PACKAGE_FILE_NAME "libfronius-${PROJECT_VERSION}")

include(CPack)