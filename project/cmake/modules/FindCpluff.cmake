find_package(EXPAT REQUIRED)

ExternalProject_ADD(libcpluff SOURCE_DIR ${CORE_SOURCE_DIR}/lib/cpluff
                    PREFIX ${CORE_BUILD_DIR}/cpluff
                    UPDATE_COMMAND ${UPDATE_COMMAND}
                    CONFIGURE_COMMAND CC=${CMAKE_C_COMPILER} ${CORE_SOURCE_DIR}/lib/cpluff/configure
                                      --disable-nls
                                      --enable-static
                                      --disable-shared
                                      --with-pic
                                      --prefix=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
                                      --host=${ARCH}
                                      CFLAGS=${defines}
                                      LDFLAGS=${ldflags})

set(CPLUFF_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/include
                        ${EXPAT_INCLUDE_DIRS})
set(CPLUFF_LIBRARIES    ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/libcpluff.a
                        ${EXPAT_LIBRARIES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Cpluff DEFAULT_MSG CPLUFF_INCLUDE_DIRS CPLUFF_LIBRARIES)

mark_as_advanced(CPLUFF_INCLUDE_DIRS CPLUFF_LIBRARIES)
