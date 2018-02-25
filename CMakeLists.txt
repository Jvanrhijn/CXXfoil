cmake_minimum_required (VERSION 2.6)
project (CXXfoil)

# set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# include headers into project
add_library(CXXfoil src/cxxfoil.cc)
target_include_directories(CXXFOIL PUBLIC headers)

# add threading library
find_package(Threads REQUIRED)
