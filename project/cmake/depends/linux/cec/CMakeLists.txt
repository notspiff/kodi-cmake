project(cec)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
externalproject_add(cec
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND autoreconf -vif
                    CONFIGURE_COMMAND <SOURCE_DIR>/configure 
                      --prefix=${OUTPUT_DIR}
                      --disable-rpi
                    BUILD_COMMAND make -C src/lib
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)
                  
install(CODE "execute_process(COMMAND make -C src/lib install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
