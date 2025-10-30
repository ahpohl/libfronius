cmake_minimum_required(VERSION 3.15)

project(libfronius
    VERSION 0.0.0 # fallback if no git tag found
    DESCRIPTION "A modern C++ library for accessing Fronius inverter and meter devices"
    LANGUAGES CXX
)

# ==========================================================
#  Version from Git tag (if available)
# ==========================================================
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(GIT_TAG)
        message(STATUS "Using version from git tag: ${GIT_TAG}")
        string(REGEX REPLACE "^v" "" PROJECT_VERSION ${GIT_TAG})
        set(PROJECT_VERSION ${PROJECT_VERSION} CACHE STRING "Version from Git" FORCE)
    endif()
else()
    message(WARNING "Git not found, using fallback project version ${PROJECT_VERSION}")
endif()

# ==========================================================
#  Global settings
# ==========================================================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ==========================================================
#  Options
# ==========================================================
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_STATIC_LIBS "Build static library" ON)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ==========================================================
#  Sources
# ==========================================================
set(LIBFRONIUS_SOURCES
    src/fronius.cpp
    src/meter.cpp
    src/inverter.cpp
)

# Optionally collect headers automatically (for IDEs)
file(GLOB_RECURSE LIBFRONIUS_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/fronius/*.h"
)

# ==========================================================
#  Shared library target
# ==========================================================
if(BUILD_SHARED_LIBS)
    add_library(fronius_shared SHARED ${LIBFRONIUS_SOURCES} ${LIBFRONIUS_HEADERS})

    target_include_directories(fronius_shared
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
               $<INSTALL_INTERFACE:include>
    )

    set_target_properties(fronius_shared PROPERTIES
        OUTPUT_NAME fronius
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
    )
    message(STATUS "Configured shared library target: fronius_shared")
endif()

# ==========================================================
#  Static library target
# ==========================================================
if(BUILD_STATIC_LIBS)
    add_library(fronius_static STATIC ${LIBFRONIUS_SOURCES} ${LIBFRONIUS_HEADERS})

    target_include_directories(fronius_static
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
               $<INSTALL_INTERFACE:include>
    )

    set_target_properties(fronius_static PROPERTIES
        OUTPUT_NAME fronius
    )
    message(STATUS "Configured static library target: fronius_static")
endif()

# ==========================================================
#  Installation
# ==========================================================
include(GNUInstallDirs)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(TARGET fronius_shared)
    install(TARGETS fronius_shared
        EXPORT froniusTargets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

if(TARGET fronius_static)
    install(TARGETS fronius_static
        EXPORT froniusTargets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

# Export targets
install(EXPORT froniusTargets
    FILE froniusTargets.cmake
    NAMESPACE fronius::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fronius
)

# ==========================================================
#  Package configuration for find_package()
# ==========================================================
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/froniusConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/froniusConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fronius
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/froniusConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/froniusConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/froniusConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fronius
)

# ==========================================================
#  CPack - Packaging configuration
# ==========================================================
include(CPack)

set(CPACK_PACKAGE_NAME "libfronius")
set(CPACK_PACKAGE_VENDOR "Alexander Pohl")
set(CPACK_PACKAGE_CONTACT "alex@ahpohl.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A C++ library to communicate with Fronius inverter and smart meter devices")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

# Supported generators
set(CPACK_GENERATOR "TGZ;DEB;RPM")

# Debian-specific
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Alexander Pohl")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libmodbus")

# RPM-specific
set(CPACK_RPM_PACKAGE_LICENSE "MIT")

message(STATUS "CPack packaging enabled: will produce .tar.gz, .deb, and .rpm packages")
