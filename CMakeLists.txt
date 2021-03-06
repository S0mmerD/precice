cmake_minimum_required (VERSION 3.5)

project(preCICE VERSION 1.3.0 LANGUAGES CXX)

# CMake Policies

# CMP0074: find_package() uses <PackageName>_ROOT variables.
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()
# CMP0075 Include file check macros honor CMAKE_REQUIRED_LIBRARIES
if(POLICY CMP0075)
    cmake_policy(SET CMP0075 NEW)
endif()


list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/tools/cmake-modules")

set (CMAKE_CXX_STANDARD 11)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
message(STATUS "Build configuration: " ${CMAKE_BUILD_TYPE})


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter")
endif()


set(Platform "none" CACHE STRING "Special configurations for certain platforms")
set_property(CACHE Platform PROPERTY STRINGS none "SuperMUC")
message(STATUS "Special platform settings: " ${Platform})
if (Platform STREQUAL "SuperMUC")
  add_definitions(-DSuperMUC_WORK)
endif()


option(BUILD_SHARED_LIBS "Build a shared library" ON)


option(MPI "Enables MPI-based communication and running coupling tests." ON)
if (MPI)
  find_package(MPI REQUIRED)
  include_directories(${MPI_INCLUDE_PATH})
  set(COMPILE_FLAGS  ${COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
  set(LINK_FLAGS ${LINK_FLAGS} ${MPI_LINK_FLAGS})
else()
  add_definitions(-DPRECICE_NO_MPI)
  message(STATUS "MPI support disabled")
endif()


option(PETSC "Enable use of the PETSc linear algebra library." ON)
if (PETSC AND MPI)
  find_package(PETSc 3.6 REQUIRED)
  include_directories(${PETSC_INCLUDES})
  set (PETSC_VERSION_MAJOR ${PETSC_VERSION_MAJOR} CACHE STRING "Petsc Major version")
  set (PETSC_VERSION_MINOR ${PETSC_VERSION_MINOR} CACHE STRING "Petsc Minor version")
else()
  add_definitions(-DPRECICE_NO_PETSC)
  message(STATUS "PETSc support disabled")
endif()

find_package (Threads REQUIRED)


find_package(Boost 1.60.0 REQUIRED
  COMPONENTS filesystem log log_setup program_options system thread unit_test_framework)
add_definitions(-DBOOST_ALL_DYN_LINK -DBOOST_ASIO_ENABLE_OLD_SERVICES)
include_directories(${Boost_INCLUDE_DIRS})


find_package(Eigen3 3.2 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DEIGEN_INITIALIZE_MATRICES_BY_NAN)
endif()


find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})


option(PYTHON "Python support" ON)
if (PYTHON)
  set(Python_ADDITIONAL_VERSIONS "2.7")
  find_package(PythonLibs 2.7 REQUIRED)
  find_package(NumPy REQUIRED)
  include_directories(${PYTHON_INCLUDE_DIRS})
  include_directories(${NumPy_INCLUDE_DIRS})
  add_definitions(-DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION)
else()
  add_definitions(-DPRECICE_NO_PYTHON)
  message(STATUS "Python support disabled")
endif()


# Create versions.hpp and add binary dir to include dirs
configure_file("${PROJECT_SOURCE_DIR}/src/versions.hpp.in" "${PROJECT_BINARY_DIR}/src/versions.hpp" @ONLY)
include_directories("${PROJECT_BINARY_DIR}/src")

add_subdirectory("src")
include_directories("src")

add_library(precice ${sourcesAllNoMain})
set(headers
  src/precice/SolverInterface.hpp
  src/precice/MeshHandle.hpp
  src/precice/Constants.hpp
  src/precice/bindings/c/SolverInterfaceC.h
  src/precice/bindings/c/Constants.h)
set_target_properties(precice PROPERTIES PUBLIC_HEADER "${headers}")

target_link_libraries(precice PRIVATE ${PYTHON_LIBRARIES})
target_link_libraries(precice PUBLIC ${MPI_LIBRARIES})
target_link_libraries(precice PUBLIC ${Boost_LIBRARIES})
target_link_libraries(precice PUBLIC ${PETSC_LIBRARIES})
target_link_libraries(precice PUBLIC ${LIBXML2_LIBRARIES})

add_executable(binprecice "src/drivers/main.cpp")
target_link_libraries(binprecice Threads::Threads)
target_link_libraries(binprecice precice)

add_executable(testprecice
  "src/testing/main.cpp" ${sourcesTests})
target_link_libraries(testprecice Threads::Threads)
target_link_libraries(testprecice precice)

install(TARGETS precice binprecice
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include/precice
  RUNTIME DESTINATION bin)
