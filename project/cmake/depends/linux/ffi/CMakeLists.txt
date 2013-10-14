project(ffi)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
externalproject_add(ffi
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND ""
                    CONFIGURE_COMMAND <SOURCE_DIR>/configure 
                      --prefix=${OUTPUT_DIR}
                      --disable-shared
                      --disable-builddir
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)

install(CODE "execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
install(DIRECTORY ${OUTPUT_DIR}/lib/libffi-3.0.11/include
        DESTINATION include/ffi)
