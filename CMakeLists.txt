cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")

add_library(cxxfoil STATIC 
  src/xfoil_config.cc
  src/xfoil_runner.cc
)

if(TESTING)
  add_subdirectory(tests)
endif()

