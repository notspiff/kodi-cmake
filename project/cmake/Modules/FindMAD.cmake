# - Try to find MAD
# Once done this will define
#
# MAD_FOUND - system has libmad
# MAD_INCLUDE_DIRS - the libmad include directory
# MAD_LIBRARIES - The libmad libraries

if(PKG_CONFIG_FOUND)
  pkg_check_modules (MAD mad)
  list(APPEND MAD_INCLUDE_DIRS ${MAD_INCLUDEDIR})
else()
  find_path(MAD_INCLUDE_DIRS mad.h)
  find_library(MAD_LIBRARIES mad)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MAD DEFAULT_MSG MAD_INCLUDE_DIRS MAD_LIBRARIES)

mark_as_advanced(MAD_INCLUDE_DIRS MAD_LIBRARIES)
