if(ENABLE_EXTERNAL_FFMPEG OR ENABLE_EXTERNAL_LIBAV)
  set(FFMPEG_PKGS libavcodec libavfilter libavformat
                  libavutil libswscale)
  if(ENABLE_EXTERNAL_FFMPEG)
    list(APPEND FFMPEG_PKGS libswresample libpostproc)
    list(APPEND FFMPEG_DEFINITIONS -DUSE_EXTERNAL_FFMPEG=1)
  endif()
  if(ENABLE_EXTERNAL_LIBAV)
    list(APPEND FFMPEG_PKGS libavresample)
    list(APPEND FFMPEG_DEFINITIONS -DUSE_EXTERNAL_LIBAV=1)
  endif()
  if(PKG_CONFIG_FOUND)
    pkg_check_modules (FFMPEG ${FFMPEG_PKGS})
  endif()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(FFMPEG DEFAULT_MSG FFMPEG_INCLUDE_DIRS FFMPEG_LIBRARIES)

  mark_as_advanced(FFMPEG_INCLUDE_DIRS FFMPEG_LIBRARIES FFMPEG_DEFINITIONS)
else()
  if(ENABLE_STATIC_FFMPEG)
    set(FFMPEG_TYPES --enable-static --disable-shared)
  else()
    set(FFMPEG_TYPES --enable-shared --disable-static)
  endif()
  set(ffmpeg_conf ${CORE_SOURCE_DIR}/lib/ffmpeg/configure
                   --disable-muxers
                   --enable-muxer=spdif
                   --enable-muxer=adts
                   --enable-muxer=asf
                   --enable-muxer=ipod
                   --disable-encoders
                   --enable-encoder=ac3
                   --enable-encoder=aac
                   --enable-encoder=wmav2
                   --disable-decoder=mpeg_xvmc
                   --disable-devices
                   --disable-ffprobe
                   --disable-ffplay
                   --disable-ffmpeg
                   --disable-crystalhd
                   ${FFMPEG_TYPES}
                   --disable-doc
                   --enable-postproc
                   --enable-gpl
                   --enable-optimizations
                   --enable-vdpau
                   --enable-protocol=http
                   --enable-pthreads
                   --enable-runtime-cpudetect
                   --enable-pic
                   --custom-libname-with-major='$(FULLNAME)-$(LIBMAJOR)'-${ARCH}${CMAKE_SHARED_MODULE_SUFFIX}
                   --prefix=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg
                   --cc=${CMAKE_C_COMPILER}
                   ${FFMPEG_OPTS})

  if(GNUTLS_FOUND)
    list(APPEND ffmpeg_conf --enable-gnutls)
  endif()

  # All this nonsense required to avoid evaluating $()
  string(REPLACE ";" " " ffmpeg_conf "${ffmpeg_conf}")
  file(WRITE ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/tmp/configure_ffmpeg ${ffmpeg_conf})
  execute_process(COMMAND chmod 755 ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/tmp/configure_ffmpeg)
  ExternalProject_ADD(ffmpeg SOURCE_DIR ${CORE_SOURCE_DIR}/lib/ffmpeg
                      PREFIX ${CORE_BUILD_DIR}/ffmpeg
                      CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/tmp/configure_ffmpeg)
  if(ENABLE_STATIC_FFMPEG)
    list(APPEND FFMPEG_DEFINITIONS -DUSE_STATIC_FFMPEG=1)
    file(WRITE ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/ffmpeg-link-wrapper
"#!/bin/bash
avformat=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs libavcodec`
avcodec=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs libavformat`
avfilter=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs libavfilter`
avutil=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs libavutil`
swscale=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs libswscale`
swresample=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs libswresample`
$@ $avcodec $avformat $avcodec $avfilter $swscale $swresample -lpostproc")
    file(COPY ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/ffmpeg-link-wrapper
         DESTINATION ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
    set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg-link-wrapper <CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
    set(CMAKE_CXX_LINK_EXECUTABLE ${CMAKE_CXX_LINK_EXECUTABLE} PARENT_SCOPE)
  else()
    set(ffmpeg_libs avcodec-54 avformat-54 avfilter-3
                    avutil-52 swscale-2 swresample-0 postproc-52)
    foreach(lib ${ffmpeg_libs})
      add_custom_command(TARGET ffmpeg POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/lib/${lib}-${ARCH}${CMAKE_SHARED_MODULE_SUFFIX} ${CMAKE_BINARY_DIR}/system/players/dvdplayer/${lib}-${ARCH}${CMAKE_SHARED_MODULE_SUFFIX})
      install(PROGRAMS ${CMAKE_BINARY_DIR}/system/players/dvdplayer/${lib}-${ARCH}${CMAKE_SHARED_MODULE_SUFFIX}
              DESTINATION lib/xbmc/system/players/dvdplayer)
    endforeach()
  endif()

  set(FFMPEG_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/include)
endif()
