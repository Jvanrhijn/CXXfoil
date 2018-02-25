project (CXXfoil)
cmake_minimum_required (VERSION 2.6)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED YES)

include_directories (headers)
add_subdirectory (src)

add_subdirectory (tests)
enable_testing()
add_test (NAME XfoilTest COMMAND Test)
