string(REPLACE ";" " " defines "${SYSTEM_DEFINES} -I${EXPAT_INCLUDE_DIR}")
get_filename_component(expat_dir ${EXPAT_LIBRARY} PATH)
set(ldflags "-L${expat_dir}")
ExternalProject_ADD(libcpluff SOURCE_DIR ${CORE_SOURCE_DIR}/lib/cpluff
                    PREFIX ${CORE_BUILD_DIR}/cpluff
                    UPDATE_COMMAND ${UPDATE_COMMAND}
                    CONFIGURE_COMMAND ${CORE_SOURCE_DIR}/lib/cpluff/configure
                                      --disable-nls
                                      --enable-static
                                      --disable-shared
                                      --with-pic
                                      --prefix=<INSTALL_DIR>
                                      CFLAGS=${defines}
                                      LDFLAGS=${ldflags})

set(CPLUFF_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/cpluff/include)
set(CPLUFF_FOUND 1)

set(ldflags "${ldflags};-lexpat")
core_link_library(${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/cpluff/lib/libcpluff.a
                  system/libcpluff libcpluff extras "${ldflags}")
set(WRAP_FILES ${WRAP_FILES} PARENT_SCOPE)

mark_as_advanced(CPLUFF_INCLUDE_DIRS CPLUFF_FOUND)
