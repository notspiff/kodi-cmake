ExternalProject_ADD(libcpluff SOURCE_DIR ${XBMC_SOURCE_DIR}/lib/cpluff
                    PREFIX ${XBMC_BUILD_DIR}/cpluff
                    UPDATE_COMMAND ${UPDATE_COMMAND}
                    CONFIGURE_COMMAND ${XBMC_SOURCE_DIR}/lib/cpluff/configure
                                      --disable-nls
                                      --enable-static
                                      --disable-shared
                                      --with-pic
                                      --prefix=<INSTALL_DIR>)

set(CPLUFF_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR}/cpluff/include)
set(CPLUFF_FOUND 1)

xbmc_link_library(${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR}/cpluff/lib/libcpluff.a
                 system/libcpluff libcpluff "extras" -lexpat)
set(WRAP_FILES ${WRAP_FILES} PARENT_SCOPE)

mark_as_advanced(CPLUFF_INCLUDE_DIRS CPLUFF_FOUND)
