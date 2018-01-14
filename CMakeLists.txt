cmake_minimum_required (VERSION 2.6)
project (CProp)

# set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# include headers into project
include_directories(headers)

# add sources
file(GLOB SOURCES "src/*.c*" "tests/*.c*" "headers/*.h")

# add the executable
add_executable(test ${SOURCES})

# add threading library
find_package(Threads REQUIRED)
target_link_libraries(test Threads::Threads)

# enable testing framework
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
target_link_libraries(test ${GTEST_BOTH_LIBRARIES})

add_test(XfoilTest test)
