cmake_minimum_required(VERSION 3.9)

add_library(cxxfoil STATIC 
  src/xfoil_config.cc
)

if(TESTING)
  add_subdirectory(tests)
endif()

