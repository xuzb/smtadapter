set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
find_package(LLVM)

if( NOT LLVM_FOUND )
  message(STATUS "Warning: The targets in unittests cannot be compiled correctly as llvm is missing. These targets have been excluded.")
  return()
endif()

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_CFLAGS})

set(LLVM_MODULE_LIBS ${LLVM_MODULE_LIBS} z pthread dl m)
find_library(TINFO_LIBRARY tinfo)
if (TINFO_LIBRARY)
  set(LLVM_MODULE_LIBS ${LLVM_MODULE_LIBS} tinfo)
endif()

add_executable(z3test z3test.cpp)
target_link_libraries(z3test ${SMTADAPTER_LIBS} ${LLVM_MODULE_LIBS} ${LLVM_LDFLAGS})

add_custom_target(unittests DEPENDS z3test)