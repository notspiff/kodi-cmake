# - Try to find FLAC
# Once done this will define
#
# FLAC_FOUND - system has libFLAC
# FLAC_INCLUDE_DIRS - the libFLAC include directory
# FLAC_LIBRARIES - The libFLAC libraries

if(PKG_CONFIG_FOUND)
  pkg_check_modules (FLAC flac)
else()
  find_path(FLAC_INCLUDE_DIRS FLAC/stream_decoder.h)
  find_library(FLAC_LIBRARIES FLAC)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FLAC DEFAULT_MSG FLAC_INCLUDE_DIRS FLAC_LIBRARIES)

mark_as_advanced(FLAC_INCLUDE_DIRS FLAC_LIBRARIES)
