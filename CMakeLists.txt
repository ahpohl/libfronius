cmake_minimum_required(VERSION 3.15)

# --- Version from git tag ---
find_package(Git QUIET)
if(Git_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" describe --tags --abbrev=0
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _git_rc
        ERROR_QUIET
    )
endif()

if(NOT _git_rc EQUAL 0 OR GIT_TAG STREQUAL "")
    set(GIT_TAG "0.0.0")
endif()

string(REGEX REPLACE "^v" "" GIT_TAG_STRIPPED "${GIT_TAG}")

project(
    fronius
    LANGUAGES CXX
    DESCRIPTION "Library to access Fronius inverters and smart meters"
    VERSION ${GIT_TAG_STRIPPED}
)

include(GNUInstallDirs)

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

# --- Link libmodbus via pkg-config ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBMODBUS REQUIRED IMPORTED_TARGET libmodbus)

# --- Shared library ---
if(BUILD_SHARED_LIBS)
    add_library(fronius_shared SHARED ${MYLIB_SOURCES})
    add_library(fronius::shared ALIAS fronius_shared)
    target_include_directories(fronius_shared 
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    target_link_libraries(fronius_shared PRIVATE PkgConfig::LIBMODBUS)
    set_target_properties(fronius_shared PROPERTIES
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
    add_library(fronius::static ALIAS fronius_static)
    target_include_directories(fronius_static
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    target_link_libraries(fronius_static PRIVATE PkgConfig::LIBMODBUS)
    set_target_properties(fronius_static PROPERTIES
        OUTPUT_NAME "fronius"
        CXX_STANDARD 23
        EXPORT_NAME static
    )
    message(STATUS "Building static library: fronius_static")
endif()

# --- Install library and headers ---
# Install headers under include/fronius so consumers can #include <fronius/...>
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fronius)

if(BUILD_SHARED_LIBS)
    install(TARGETS fronius_shared
        EXPORT froniusTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

if(BUILD_STATIC_LIBS)
    install(TARGETS fronius_static
        EXPORT froniusTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

# --- CMake Package Config ---
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/froniusConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/froniusConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fronius
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
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fronius
)

install(
    EXPORT froniusTargets
    NAMESPACE fronius::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fronius
)

# --- pkg-config (.pc) generation ---
set(FRONIUS_PC_REQUIRES_PRIVATE "libmodbus")
set(FRONIUS_PC_LIBS_PRIVATE "")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/fronius.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/fronius.pc
    @ONLY
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/fronius.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# --- CPack (components) ---
include(InstallRequiredSystemLibraries)

# Generate component packages for DEB
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL runtime devel)

# Global metadata
set(CPACK_GENERATOR "TGZ;DEB")
set(CPACK_PACKAGE_NAME "libfronius")
set(CPACK_PACKAGE_CONTACT "Alexander Pohl <alex@ahpohl.com>")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

# Per-component Debian package names and metadata
get_target_property(FRONIUS_ABI fronius_shared SOVERSION)
set(CPACK_DEBIAN_RUNTIME_PACKAGE_NAME "libfronius${FRONIUS_ABI}")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_SECTION "libs")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_SHLIBDEPS ON)

set(CPACK_DEBIAN_DEVEL_PACKAGE_NAME "libfronius-dev")
set(CPACK_DEBIAN_DEVEL_PACKAGE_SECTION "libdevel")
# Make -dev depend on the exact same runtime version
set(CPACK_DEBIAN_DEVEL_PACKAGE_DEPENDS "${CPACK_DEBIAN_RUNTIME_PACKAGE_NAME} (= ${PROJECT_VERSION})")

# Optional descriptions
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Library to access Fronius inverters and smart meters")
set(CPACK_COMPONENT_DEVEL_DESCRIPTION   "Library to access Fronius inverters and smart meters (development files)")

# DEB naming: canonical names per Debian policy
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

# TGZ custom archive name
string(TOLOWER "${CMAKE_SYSTEM_NAME}" OS_NAME)
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" ARCH)
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_v${PROJECT_VERSION}_${OS_NAME}_${ARCH}")

include(CPack)