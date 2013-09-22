project(mad)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
externalproject_add(mad
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND touch NEWS AUTHORS ChangeLog && 
                                   sed -ie "s|libmad.list|libmad.list mad.pc|g"
                                   ${CMAKE_CURRENT_SOURCE_DIR}/configure.ac &&
                                   autoreconf -vif
                                   WORKING_DIRECTORY <SOURCE_DIR>
                                   CONFIGURE_COMMAND sed -i "/-fforce-mem/d"
                                                     <SOURCE_DIR>/configure &&
                                                     <SOURCE_DIR>/configure 
                                                       --prefix=${OUTPUT_DIR}
                                                       --enable-fpm=default
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)
                  
install(CODE "execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
