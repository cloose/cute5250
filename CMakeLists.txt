cmake_minimum_required(VERSION 2.8.12)

project(cute5250)

# Set version number
set(CUTE5250_MAJOR_VERSION 0)
set(CUTE5250_MINOR_VERSION 1)
set(CUTE5250_PATCH_VERSION 0)
set(CUTE5250_VERSION ${CUTE5250_MAJOR_VERSION}.${CUTE5250_MINOR_VERSION}.${CUTE5250_PATCH_VERSION})

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Enable CTest
enable_testing()

# Add our custom cmake modules
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
message(STATUS "Module path: ${CMAKE_MODULE_PATH}")

### C++11 Support ###

find_package(CXXFeatures)

set(needed_features
    CXXFeatures_auto
    CXXFeatures_initializer_list
)

foreach(i ${needed_features})
    if(NOT ${i}_FOUND)
        message(FATAL_ERROR "CXX feature \"${i}\" is not supported by the compiler")
    endif()
endforeach()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_COMPILER_FLAGS}")
message(STATUS "C++ Flags: ${CMAKE_CXX_FLAGS}")

### Build Type ###

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

### Build Options ###

option(BUILD_WITH_CODE_COVERAGE "Enable code coverage with gconv" OFF)

### Dependencies ###

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Widgets REQUIRED)

### Subdirs ###

add_subdirectory(lib)
#add_subdirectory(src)
add_subdirectory(test)
