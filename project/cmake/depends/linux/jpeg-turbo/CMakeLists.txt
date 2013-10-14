project(jpeg-turbo)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
externalproject_add(jpeg-turbo
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND ""
                    CONFIGURE_COMMAND CFLAGS=-O3 <SOURCE_DIR>/configure 
                        --prefix=${OUTPUT_DIR}
                        --with-jpeg8
                        --disable-shared
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)
                  
install(CODE "execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
