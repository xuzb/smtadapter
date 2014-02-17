cmake_minimum_required(VERSION 2.8)
include(ExternalProject)

ExternalProject_Add(z3
   SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/z3
   BUILD_IN_SOURCE 1
   GIT_REPOSITORY https://git01.codeplex.com/z3
   GIT_TAG unstable
   UPDATE_COMMAND ""
   CONFIGURE_COMMAND  python <SOURCE_DIR>/scripts/mk_make.py --staticlib
   BUILD_COMMAND  $(MAKE) -C <SOURCE_DIR>/build libz3.a
   INSTALL_COMMAND ln -f -s <SOURCE_DIR>/build/libz3.a <SOURCE_DIR>/libz3.a
)
