project(afpfs-ng)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
externalproject_add(afpfs-ng
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND autoreconf -vif WORKING_DIRECTORY <SOURCE_DIR>
                    CONFIGURE_COMMAND <SOURCE_DIR>/configure --disable-fuse --prefix=${OUTPUT_DIR}
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)
                  
install(CODE "execute_process(COMMAND make -C lib install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND make -C include install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
