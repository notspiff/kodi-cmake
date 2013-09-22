# - Try to find libsamplerate
# Once done this will define
#
# SAMPLERATE_FOUND - system has libsamplerate
# SAMPLERATE_INCLUDE_DIRS - the libsamplerate include directory
# SAMPLERATE_LIBRARIES - The libsamplerate libraries

if(PKG_CONFIG_FOUND)
  pkg_check_modules (SAMPLERATE samplerate)
  if(NOT SAMPLERATE_INCLUDE_DIRS)
    set(SAMPLERATE_INCLUDE_DIRS ${SAMPLERATE_INCLUDEDIR})
  endif()
else()
  find_path(SAMPLERATE_INCLUDE_DIRS samplerate.h)
  find_library(SAMPLERATE_LIBRARIES samplerate)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SAMPLERATE DEFAULT_MSG SAMPLERATE_INCLUDE_DIRS SAMPLERATE_LIBRARIES)

mark_as_advanced(SAMPLERATE_INCLUDE_DIRS SAMPLERATE_LIBRARIES)
