cmake_minimum_required(VERSION 3.15)
project(libfronius VERSION 1.0.0 LANGUAGES CXX
   DESCRIPTION "A C++ library to access Fronius devices")

# Ensure position independent code for shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Options to control which libraries are built
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" ON)

# Output directories for built binaries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Common sources
set(MYLIB_SOURCES
    src/fronius.cpp
    src/meter.cpp
    src/inverter.cpp
)

if(BUILD_SHARED_LIBS)
    add_library(mylib_shared SHARED ${MYLIB_SOURCES})
    
    target_include_directories(mylib_shared
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    ) 

    # Set library versioning
    set_target_properties(mylib_shared PROPERTIES
        OUTPUT_NAME "fronius"
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        CXX_STANDARD 23
    )
    message(STATUS "Building shared library: mylib_shared")
endif()

# --- Static library
if(BUILD_STATIC_LIBS)
    add_library(mylib_static STATIC ${MYLIB_SOURCES} ${MYLIB_HEADERS})

    target_include_directories(mylib_static
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    ) 

    # Give the static lib a nice name
    set_target_properties(mylib_static PROPERTIES
        OUTPUT_NAME "fronius"
        CXX_STANDARD 23
    )
    message(STATUS "Building static library: mylib_static")
endif()

# --- Install rules
install(DIRECTORY include/ DESTINATION include)
if(BUILD_SHARED_LIBS)
    install(TARGETS mylib_shared DESTINATION lib)
endif()
if(BUILD_STATIC_LIBS)
    install(TARGETS mylib_static DESTINATION lib)
endif()
