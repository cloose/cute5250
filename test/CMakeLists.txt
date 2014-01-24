### Dependencies ###

include_directories($ENV{GMOCK_HOME}/include $ENV{GMOCK_HOME}/gtest/include)
link_directories($ENV{GMOCK_HOME}/build $ENV{GMOCK_HOME}/gtest/build)

find_package(Qt5Test REQUIRED)

### Subdirs ###

add_subdirectory(unit)

