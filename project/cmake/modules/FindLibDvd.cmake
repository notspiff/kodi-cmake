string(REPLACE ";" " " defines "${CMAKE_C_FLAGS} ${SYSTEM_DEFINES}")
if(ENABLE_DVDCSS)
  ExternalProject_ADD(dvdcss SOURCE_DIR ${CORE_SOURCE_DIR}/lib/libdvd/libdvdcss/
                      PREFIX ${CORE_BUILD_DIR}/libdvd
                      UPDATE_COMMAND ${UPDATE_COMMAND}
                      CONFIGURE_COMMAND <SOURCE_DIR>/configure
                                        --disable-doc
                                        --enable-static
                                        --disable-shared
                                        --with-pic
                                        --prefix=<INSTALL_DIR>
                                        --host=${ARCH}
                                        "CC=${CMAKE_C_COMPILER}"
                                        "CFLAGS=${defines}")

  core_link_library(${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/libdvd/lib/libdvdcss.a
                    system/players/dvdplayer/libdvdcss dvdcss)
endif()

set(DVDREAD_CFLAGS "-D_XBMC")
if(ENABLE_DVDCSS)
  set(DVDREAD_CFLAGS "${DVDREAD_CFLAGS} -DHAVE_DVDCSS_DVDCSS_H -I${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/libdvd/include")
endif(ENABLE_DVDCSS)

ExternalProject_ADD(dvdread SOURCE_DIR ${CORE_SOURCE_DIR}/lib/libdvd/libdvdread/
                    PREFIX ${CORE_BUILD_DIR}/libdvd
                    UPDATE_COMMAND ${UPDATE_COMMAND}
                    CONFIGURE_COMMAND  <SOURCE_DIR>/configure
                                      --enable-static
                                      --disable-shared
                                      --with-pic
                                      --prefix=<INSTALL_DIR>
                                      --host=${ARCH}
                                      "CC=${CMAKE_C_COMPILER}"
                                      "CFLAGS=${defines} ${DVDREAD_CFLAGS}")
if(ENABLE_DVDCSS)
  add_dependencies(dvdread dvdcss)
endif()

core_link_library(${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/libdvd/lib/libdvdread.a
                  system/players/dvdplayer/libdvdread dvdread)

ExternalProject_ADD(dvdnav SOURCE_DIR ${CORE_SOURCE_DIR}/lib/libdvd/libdvdnav
                    PREFIX ${CORE_BUILD_DIR}/libdvd
                    UPDATE_COMMAND ${UPDATE_COMMAND}
                    CONFIGURE_COMMAND  <SOURCE_DIR>/configure2
                                      --disable-shared
                                      --enable-static
                                      --with-pic
                                      --host=${ARCH}
                                      --prefix=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/libdvd
                                      --with-dvdread-config=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/libdvd/bin/dvdread-config
                                      "LDFLAGS=${AC_LDFLAGS} -L${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/libdvd/lib"
                                      "CFLAGS=${defines} ${DVDREAD_CFLAGS}"
                                      "LIBS=-ldvdcss"
                                      "CC=${CMAKE_C_COMPILER}")
add_dependencies(dvdnav dvdread)
core_link_library(${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/libdvd/lib/libdvdnav.a
                  system/players/dvdplayer/libdvdnav dvdnav)

set(dvdnav_internal_headers libdvdnav/src/dvdnav_internal.h
                            libdvdnav/src/remap.h
                            libdvdnav/src/vm/vm.h
                            libdvdnav/src/vm/decoder.h)

foreach(dvdnav_header ${dvdnav_internal_headers})
  get_filename_component(header ${dvdnav_header} NAME)
  file(COPY ${CORE_SOURCE_DIR}/lib/libdvd/${dvdnav_header}
       DESTINATION ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/libdvd/include/dvdnav)
  list(APPEND dvdnav_headers include/dvdnav/${header})
endforeach()

set(LIBDVD_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/libdvd/include)
set(LIBDVD_FOUND 1)
