project(boost)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
externalproject_add(boost
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND ""
                    CONFIGURE_COMMAND ./bootstrap.sh --prefix=${OUTPUT_DIR}
                    BUILD_COMMAND ./bjam --prefix=${OUTPUT_DIR} toolset=gcc
                    #                                         --toolset-root=$(TOOLCHAIN)/$(HOST)/bin
                                         --disable-icu
	                                 --without-date_time
                                         --without-filesystem
                                         --without-graph
                                         --without-graph_parallel
                                         --without-iostreams
                                         --without-math
                                         --without-mpi
                                         --without-program_options
                                         --without-python
                                         --without-random
                                         --without-regex
                                         --without-serialization
                                         --without-signals
                                         --without-system
                                         --without-test
                                         --without-thread
                                         --without-wave install
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)
                  
install(CODE "message(\"Boost installed\")")
