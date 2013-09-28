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
                   --enable-shared
                   --disable-static
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
                   --prefix=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg)

  # All this nonsense required to avoid evaluating $()
  string(REPLACE ";" " " ffmpeg_conf "${ffmpeg_conf}")
  file(WRITE ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/tmp/configure_ffmpeg ${ffmpeg_conf})
  execute_process(COMMAND chmod 755 ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/tmp/configure_ffmpeg)
  ExternalProject_ADD(ffmpeg SOURCE_DIR ${CORE_SOURCE_DIR}/lib/ffmpeg
                      PREFIX ${CORE_BUILD_DIR}/ffmpeg
                      CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/tmp/configure_ffmpeg)
  set(ffmpeg_libs avcodec-54 avformat-54 avfilter-3
                  avutil-52 swscale-2 swresample-0 postproc-52)
  foreach(lib ${ffmpeg_libs})
    add_custom_command(TARGET ffmpeg POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/lib/${lib}-${ARCH}${CMAKE_SHARED_MODULE_SUFFIX} ${CMAKE_BINARY_DIR}/system/players/dvdplayer/${lib}-${ARCH}${CMAKE_SHARED_MODULE_SUFFIX})
    install(PROGRAMS ${CMAKE_BINARY_DIR}/system/players/dvdplayer/${lib}-${ARCH}${CMAKE_SHARED_MODULE_SUFFIX}
            DESTINATION lib/xbmc/system/players/dvdplayer)
  endforeach()

  set(FFMPEG_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/include)
endif()
