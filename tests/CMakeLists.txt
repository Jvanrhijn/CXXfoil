# add dependencies
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
include_directories(
                    ${GTEST_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}/headers
                    )

add_executable(Test tests.cc)
target_link_libraries(
                      Test 
                      Threads::Threads
                      ${GTEST_BOTH_LIBRARIES}
                      CXXfoil
                      )
