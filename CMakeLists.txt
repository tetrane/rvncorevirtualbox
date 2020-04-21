cmake_minimum_required(VERSION 3.7)
project(rvncorevirtualbox)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

option(BUILD_SHARED_LIBS "Set to ON to build shared libraries; OFF for static libraries." OFF)
option(WARNING_AS_ERROR "Set to ON to build with -Werror" ON)

option(BUILD_TEST_COVERAGE "Set to ON to build while generating coverage information. Will put source on the build directory." OFF)

add_library(rvncorevirtualbox
  src/core_virtualbox.cpp
  src/cpu_virtualbox.cpp
  src/memory_chunk.cpp
  src/memory_virtualbox.cpp
  src/physical_memory.cpp
)

target_compile_options(rvncorevirtualbox PRIVATE -W -Wall -Wextra -Wmissing-include-dirs -Wunknown-pragmas -Wpointer-arith -Wmissing-field-initializers -Wno-multichar -Wreturn-type)

if(WARNING_AS_ERROR)
  target_compile_options(rvncorevirtualbox PRIVATE -Werror)
endif()

if(BUILD_TEST_COVERAGE)
  target_compile_options(rvncorevirtualbox PRIVATE -g -O0 --coverage -fprofile-arcs -ftest-coverage)
  target_link_libraries(rvncorevirtualbox PRIVATE gcov)
endif()

target_include_directories(rvncorevirtualbox
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

set(PUBLIC_HEADERS
  include/core_virtualbox.h
  include/core_virtualbox_def.h
  include/cpu_virtualbox.h
  include/memory_chunk.h
  include/memory_virtualbox.h
  include/physical_memory.h
)

set_target_properties(rvncorevirtualbox PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
  POSITION_INDEPENDENT_CODE ON
)

include(GNUInstallDirs)
install(TARGETS rvncorevirtualbox
  EXPORT rvncorevirtualbox-export
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(EXPORT rvncorevirtualbox-export
  FILE
    rvncorevirtualbox-config.cmake
  DESTINATION
    ${CMAKE_INSTALL_DATADIR}/cmake/rvncorevirtualbox
)

add_subdirectory(bin)

enable_testing()
add_subdirectory(test)
