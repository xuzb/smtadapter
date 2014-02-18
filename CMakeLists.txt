if( CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR )
  project(solver)
  cmake_minimum_required(VERSION 2.8)
  set(CMAKE_VERBOSE_MAKEFILE on)
  set(IS_TOP_LEVEL TRUE)

  # target etags/tags
  if (UNIX)
    ADD_CUSTOM_TARGET(etags etags --members --declarations  `find . -name '*.cc' -or -name '*.hh' -or -name '*.cpp' -or -name '*.h' -or -name '*.c'`)
  endif (UNIX)
endif()

# Z3 depends on exceptions.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fexceptions")

set(SOLVER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(SMT_LIBS_DIR ${SOLVER_SOURCE_DIR}/lib)

# Z3 libraries
set(Z3_LIBS ${SMT_LIBS_DIR}/z3/libz3.a pthread gomp rt)

# Z3 include directories
set(Z3_INCLUDES   
  ${SMT_LIBS_DIR}/z3/src/api
  ${SMT_LIBS_DIR}/z3/src/api/c++)

# Boolector libraries
set(BOOLECTOR_DIR ${SMT_LIBS_DIR}/boolector)
set(BOOLECTOR_LIBS
  ${BOOLECTOR_DIR}/libboolector.a
  ${BOOLECTOR_DIR}/liblgl.a
  ${BOOLECTOR_DIR}/libminisat.a
  z m)

# Boolector include directories
set(BOOLECTOR_INCLUDES
  ${BOOLECTOR_DIR}/boolector)

set(SMT_LIBS ${Z3_LIBS} ${BOOLECTOR_LIBS})

add_library(solver SolverAdapter.cpp Z3Adapter.cpp)

add_dependencies(solver z3 boolector)

if(IS_TOP_LEVEL)
  set(SOLVER_LIBS solver ${SMT_LIBS})
else()
  set(SOLVER_LIBS solver ${SMT_LIBS} PARENT_SCOPE)
endif()


include_directories(
  ${Z3_INCLUDES}
  ${BOOLECTOR_INCLUDES}
  ${SOLVER_SOURCE_DIR}/include)

add_subdirectory(${SMT_LIBS_DIR})
add_subdirectory(unittests)
