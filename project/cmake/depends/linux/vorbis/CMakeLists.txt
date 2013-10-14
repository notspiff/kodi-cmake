project(vorbis)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
externalproject_add(vorbis
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND sed -ie "s|SUBDIRS = m4 include vq lib examples test doc|SUBDIRS = m4 include lib|" ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.in
                    CONFIGURE_COMMAND <SOURCE_DIR>/configure 
                      --prefix=${OUTPUT_DIR}
                      --disable-oggtest
                      --disable-docs
                      --disable-examples
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)
                  
install(CODE "execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
