# - Try to find modplug
# Once done this will define
#
# MODPLUG_FOUND - system has libmodplug
# MODPLUG_INCLUDE_DIRS - the libmodplug include directory
# MODPLUG_LIBRARIES - The libmodplug libraries

if(PKG_CONFIG_FOUND)
  pkg_check_modules (MODPLUG libmodplug)
else()
  find_path(MODPLUG_INCLUDE_DIRS libmodplug/modplug.h)
  find_library(MODPLUG_LIBRARIES modplug)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MODPLUG DEFAULT_MSG MODPLUG_INCLUDE_DIRS MODPLUG_LIBRARIES)

mark_as_advanced(MODPLUG_INCLUDE_DIRS MODPLUG_LIBRARIES)
