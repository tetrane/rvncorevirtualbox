cmake_minimum_required(VERSION 3.7)
project(test)

find_package(Boost 1.49 COMPONENTS
    unit_test_framework
    iostreams
)

if(NOT Boost_FOUND)
  message(WARNING "Boost not found, don't build tests")
  return()
endif(NOT Boost_FOUND)

set(REVEN_TEST_DATA "/reven_test_data" CACHE STRING "The path to the test's data" FORCE)

add_executable(test_memory_physical
  test_memory_physical.cpp
)

target_link_libraries(test_memory_physical
  PUBLIC
    Boost::boost

  PRIVATE
    rvncorevirtualbox
    Boost::unit_test_framework
    Boost::iostreams
)

target_compile_definitions(test_memory_physical PRIVATE "BOOST_TEST_DYN_LINK")
target_compile_definitions(test_memory_physical PRIVATE "REVEN_TEST_DATA=\"${REVEN_TEST_DATA}\"")

add_test(rvncorevirtualbox test_memory_physical)

add_executable(test_read_core
  test_read_core.cpp
)

target_link_libraries(test_read_core
  PUBLIC
    Boost::boost

  PRIVATE
    rvncorevirtualbox
    Boost::unit_test_framework
    Boost::iostreams
)

target_compile_definitions(test_read_core PRIVATE "BOOST_TEST_DYN_LINK")
target_compile_definitions(test_read_core PRIVATE "REVEN_TEST_DATA=\"${REVEN_TEST_DATA}\"")

add_test(rvncorevirtualbox test_read_core)
