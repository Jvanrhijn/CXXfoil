cmake_minimum_required(VERSION 3.9)
set(UNIT_TESTS unit_tests)

find_package(Threads REQUIRED)

set(test_files
  test.cc)

add_executable(${UNIT_TESTS} ${test_files})
target_link_libraries(${UNIT_TESTS} PRIVATE gtest Threads::Threads cxxfoil)
