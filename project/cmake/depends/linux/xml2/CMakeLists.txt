project(xml2)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
externalproject_add(xml2
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND sed -ie "s|runtest||" ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.in &&
                                   sed -ie "s|testrecurse||" ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.in
                    CONFIGURE_COMMAND <SOURCE_DIR>/configure 
                      --prefix=${OUTPUT_DIR}
                      --without-python
                      --without-iconv
                      --disable-shared
                      --with-pic
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)
                  
install(CODE "execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
