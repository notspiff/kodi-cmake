project(autoconf)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
externalproject_add(autoconf
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND ""
                    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${OUTPUT_DIR}
                    BUILD_COMMAND make
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)
                  
install(CODE "execute_process(COMMAND make install PREFIX=${OUTPUT_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
