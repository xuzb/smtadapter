cmake_minimum_required(VERSION 2.8)
include(ExternalProject)


if (CMAKE_BUILD_TYPE STREQUAL Debug)
set(Z3_MK_MAKE_CMD python scripts/mk_make.py --staticlib -d)
else ()
  set(Z3_MK_MAKE_CMD python scripts/mk_make.py --staticlib)
endif ()

ExternalProject_Add(z3
   SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/z3
   BUILD_IN_SOURCE 1
   GIT_REPOSITORY https://github.com/xuzb/z3static
   GIT_TAG master
   UPDATE_COMMAND ""
   CONFIGURE_COMMAND autoconf COMMAND ./configure COMMAND ${Z3_MK_MAKE_CMD}
   BUILD_COMMAND  $(MAKE) -C <SOURCE_DIR>/build libz3.a libz3.so
   INSTALL_COMMAND ln -f -s <SOURCE_DIR>/build/libz3.a <SOURCE_DIR>/libz3.a
)

ExternalProject_Add(boolector
   SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/boolector
   BUILD_IN_SOURCE 1
   URL http://fmv.jku.at/boolector/boolector-1.6.0-with-sat-solvers.tar.gz
   DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/download
   UPDATE_COMMAND ""
   CONFIGURE_COMMAND ""
   BUILD_COMMAND  $(MAKE)
   INSTALL_COMMAND ln -f -s ./boolector/libboolector.a ./libboolector.a
           COMMAND ln -f -s ./minisat/build/release/lib/libminisat.a ./libminisat.a
           COMMAND ln -f -s ./lingeling/liblgl.a ./liblgl.a
)
