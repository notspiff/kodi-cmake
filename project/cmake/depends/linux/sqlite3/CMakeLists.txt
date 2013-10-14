project(sqlite3)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
externalproject_add(sqlite3
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND ""
                    CONFIGURE_COMMAND CFLAGS=-DSQLITE_TEMP_STORE=3
                                      CXXFLAGS=-DSQLITE_ENABLE_COLUMN_METADATA=1
                                      TCLLIBDIR=/dev/null
                                      <SOURCE_DIR>/configure 
                      --prefix=${OUTPUT_DIR}
                      --disable-shared
                      --enable-threadsafe
                      --disable-tcl
                      --disable-readline
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)
                  
install(CODE "execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
