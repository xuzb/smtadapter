cmake_minimum_required(VERSION 2.8)
include(ExternalProject)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set(CONFIG_CMD python <SOURCE_DIR>/scripts/mk_make.py --staticlib -d)
else ()
  set(CONFIG_CMD python <SOURCE_DIR>/scripts/mk_make.py --staticlib)
endif ()

ExternalProject_Add(z3
   SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/z3
   BUILD_IN_SOURCE 1
   GIT_REPOSITORY https://git01.codeplex.com/z3
   GIT_TAG unstable
   UPDATE_COMMAND ""
   CONFIGURE_COMMAND  ${CONFIG_CMD}
   BUILD_COMMAND  $(MAKE) -C <SOURCE_DIR>/build libz3.a libz3.so
   INSTALL_COMMAND ln -f -s <SOURCE_DIR>/build/libz3.a <SOURCE_DIR>/libz3.a
)
