cmake_minimum_required(VERSION 2.8)
include(ExternalProject)


if (CMAKE_BUILD_TYPE STREQUAL Debug)
set(MK_MAKE_CMD python scripts/mk_make.py --staticlib -d)
else ()
  set(MK_MAKE_CMD python scripts/mk_make.py --staticlib)
endif ()

ExternalProject_Add(z3
   SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/z3
   BUILD_IN_SOURCE 1
   GIT_REPOSITORY https://git01.codeplex.com/forks/xuzb/z3static
   GIT_TAG master
   UPDATE_COMMAND ""
   CONFIGURE_COMMAND autoconf COMMAND ./configure COMMAND ${MK_MAKE_CMD}
   BUILD_COMMAND  $(MAKE) -C <SOURCE_DIR>/build libz3.a libz3.so
   INSTALL_COMMAND ln -f -s <SOURCE_DIR>/build/libz3.a <SOURCE_DIR>/libz3.a
)
