if(ENABLE_INTERNAL_FFMPEG)
  file(WRITE ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/ffmpeg-link-wrapper
"#!/bin/bash
if [[ $@ == *${APP_NAME_LC}.bin* || $@ == *${APP_NAME_LC}-test* ]]
then
  avformat=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs libavcodec`
  avcodec=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs libavformat`
  avfilter=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs libavfilter`
  avutil=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs libavutil`
  swscale=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs libswscale`
  swresample=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs libswresample`
  $@ $avcodec $avformat $avcodec $avfilter $swscale $swresample -lpostproc
else
  $@
fi")
  file(COPY ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/ffmpeg-link-wrapper
       DESTINATION ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
       FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
  set(FFMPEG_LINK_EXECUTABLE "${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg-link-wrapper <CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>" PARENT_SCOPE)
  set(FFMPEG_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/include)
  file(STRINGS ${PROJECT_SOURCE_DIR}/bootstrap/installdata/lib/optional/linux/ffmpeg/ffmpeg.txt def)
  string(REPLACE " " ";" def ${def})
  list(GET def 2 hash)
  list(APPEND FFMPEG_DEFINITIONS -DFFMPEG_VER_SHA=\"${hash}\"
                                 -DUSE_STATIC_FFMPEG=1)
  set(FFMPEG_FOUND 1)
else()
  set(FFMPEG_PKGS libavcodec>=56.26.100 libavfilter>=5.11.100 libavformat>=56.25.101
                  libavutil>=54.20.100 libswscale>=3.1.101 libswresample>=1.1.100 libpostproc>=53.3.100)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules (FFMPEG ${FFMPEG_PKGS})
  endif()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(FFMPEG DEFAULT_MSG FFMPEG_INCLUDE_DIRS FFMPEG_LIBRARIES)
  set(FFMPEG_FOUND 1)
endif()

mark_as_advanced(FFMPEG_INCLUDE_DIRS FFMPEG_LIBRARIES FFMPEG_DEFINITIONS FFMPEG_FOUND)
