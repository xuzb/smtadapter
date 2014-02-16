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

# As we don't depend on llvm, we need to use rtti and exceptions.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -fexceptions")

set(SOLVER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(SMT_LIBS_DIR ${SOLVER_SOURCE_DIR}/lib)

# Z3 libraries
set(Z3_LIBS ${SMT_LIBS_DIR}/z3/libz3.a pthread rt)

set(SMT_LIBS ${Z3_LIBS})

add_library(solver Z3Adapter.cpp)

add_dependencies(solver z3)

if(IS_TOP_LEVEL)
  set(SOLVER_LIBS solver ${SMT_LIBS})
else()
  set(SOLVER_LIBS solver ${SMT_LIBS} PARENT_SCOPE)
endif()

include_directories(
  ${SMT_LIBS_DIR}/z3/src/api
  ${SMT_LIBS_DIR}/z3/src/api/c++
  ${SOLVER_SOURCE_DIR}/include)

add_subdirectory(${SMT_LIBS_DIR})
add_subdirectory(unittests)
