set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
find_package(LLVM REQUIRED)

if( NOT LLVM_FOUND )
  message(FATAL_ERROR "LLVM package can't be found. Set CMAKE_PREFIX_PATH variable to LLVM's installation prefix.")
endif()

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_CFLAGS})

find_library(TINFO_LIBRARY tinfo)
if (TINFO_LIBRARY)
  set(LLVM_MODULE_LIBS ${LLVM_MODULE_LIBS} tinfo)
endif()

add_executable(z3test z3test.cpp)
target_link_libraries(z3test ${SOLVER_LIBS} ${LLVM_MODULE_LIBS} ${LLVM_LDFLAGS} ${SYSTEM_SOLVER_LIBS})