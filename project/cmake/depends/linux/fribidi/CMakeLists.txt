project(fribidi)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
externalproject_add(fribidi
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND ""
                    CONFIGURE_COMMAND CFLAGS=-D__STDC_INT64__ 
                      <SOURCE_DIR>/configure 
                        --prefix=${OUTPUT_DIR}
                        --disable-shared
                        --disable-docs
                        --enable-malloc
                        --with-glib=no
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)
                  
install(CODE "execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
