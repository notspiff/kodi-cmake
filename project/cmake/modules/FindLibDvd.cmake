if(ENABLE_DVDCSS)
  ExternalProject_ADD(dvdcss SOURCE_DIR ${XBMC_SOURCE_DIR}/lib/libdvd/libdvdcss/
                      PREFIX ${XBMC_BUILD_DIR}/libdvd
                      UPDATE_COMMAND autoreconf -vif
                      CONFIGURE_COMMAND <SOURCE_DIR>/configure
                                        --disable-doc
                                        --enable-static
                                        --disable-shared
                                        --with-pic
                                        --prefix=<INSTALL_DIR>)

  xbmc_link_library(${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR}/libdvd/lib/libdvdcss.a
                    system/players/dvdplayer/libdvdcss dvdcss)
endif()

set(DVDREAD_CFLAGS "-D_XBMC")
if(ENABLE_DVDCSS)
  set(DVDREAD_CFLAGS "${DVDREAD_CFLAGS} -DHAVE_DVDCSS_DVDCSS_H -I${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR}/libdvd/include")
endif(ENABLE_DVDCSS)

ExternalProject_ADD(dvdread SOURCE_DIR ${XBMC_SOURCE_DIR}/lib/libdvd/libdvdread/
                    PREFIX ${XBMC_BUILD_DIR}/libdvd
                    UPDATE_COMMAND autoreconf -vif
                    CONFIGURE_COMMAND <SOURCE_DIR>/configure
                                      --enable-static
                                      --disable-shared
                                      --with-pic
                                      --prefix=<INSTALL_DIR>
                                      "CFLAGS=${DVDREAD_CFLAGS}")
if(ENABLE_DVDCSS)
  add_dependencies(dvdread dvdcss)
endif()

xbmc_link_library(${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR}/libdvd/lib/libdvdread.a
                  system/players/dvdplayer/libdvdread dvdread)

ExternalProject_ADD(dvdnav SOURCE_DIR ${XBMC_SOURCE_DIR}/lib/libdvd/libdvdnav/
                    PREFIX ${XBMC_BUILD_DIR}/libdvd
                    UPDATE_COMMAND autoreconf -vif
                    CONFIGURE_COMMAND <SOURCE_DIR>/configure
                                      --disable-shared
                                      --enable-static
                                      --with-pic
                                      --prefix=${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR}/libdvd
                                      --with-dvdread-config=${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR}/libdvd/bin/dvdread-config
                                      "LDFLAGS=-L${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR}/libdvd/lib"
                                      "CFLAGS=${DVDREAD_CFLAGS}"
                                      "LIBS=-ldvdcss")
add_dependencies(dvdnav dvdread)
xbmc_link_library(${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR}/libdvd/lib/libdvdnav.a
                  system/players/dvdplayer/libdvdnav dvdnav)
#set(WRAP_FILES ${WRAP_FILES} PARENT_SCOPE)

set(dvdnav_internal_headers ${XBMC_SOURCE_DIR}/lib/libdvd/libdvdnav/src/dvdnav_internal.h
                            ${XBMC_SOURCE_DIR}/lib/libdvd/libdvdnav/src/vm/vm.h)

foreach(dvdnav_header ${dvdnav_internal_headers})
 add_custom_command(TARGET dvdnav
                   COMMAND cmake -E copy ${dvdnav_header}
                           ${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR}/libdvd/include/dvdnav)
endforeach()

set(LIBDVD_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR}/libdvd/include)
set(LIBDVD_FOUND 1)
