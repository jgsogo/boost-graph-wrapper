project(boost-graph-wrapper)
cmake_minimum_required(VERSION 2.8.12)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package (Threads REQUIRED)
find_package (Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

include_directories (${TEST_SOURCE_DIR}/src
                     ${Boost_INCLUDE_DIRS}
                    )
#add_definitions (-DBOOST_TEST_DYN_LINK)


# Copy test data
SET(TEST_DATA_PATH ${CMAKE_CURRENT_LIST_DIR}/data)
file(MAKE_DIRECTORY ${TEST_DATA_PATH})
configure_file(config_tests.h.cmake ${CMAKE_CURRENT_LIST_DIR}/config_tests.h)

# Add subdirectories with tests

