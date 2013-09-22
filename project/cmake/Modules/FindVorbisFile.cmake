# - Try to find ogg
# Once done this will define
#
# VORBISFILE_FOUND - system has libvorbisfile
# VORBISFILE_INCLUDE_DIRS - the libvorbisfile include directory
# VORBISFILE_LIBRARIES - The libvorbisfile libraries

if(PKG_CONFIG_FOUND)
  pkg_check_modules (VORBISFILE vorbisfile)
  list(APPEND VORBISFILE_INCLUDE_DIRS ${VORBISFILE_INCLUDEDIR})
else()
  find_path(VORBISFILE_INCLUDE_DIRS vorbis/vorbisfile.h)
  find_library(VORBISFILE_LIBRARIES vorbisfile)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VORBISFILE DEFAULT_MSG VORBISFILE_INCLUDE_DIRS VORBISFILE_LIBRARIES)

mark_as_advanced(VORBISFILE_INCLUDE_DIRS VORBISFILE_LIBRARIES)
