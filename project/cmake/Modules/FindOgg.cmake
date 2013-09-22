# - Try to find ogg
# Once done this will define
#
# OGG_FOUND - system has libogg
# OGG_INCLUDE_DIRS - the libogg include directory
# OGG_LIBRARIES - The libogg libraries

if(PKG_CONFIG_FOUND)
  pkg_check_modules (OGG ogg)
  list(APPEND OGG_INCLUDE_DIRS ${OGG_INCLUDEDIR})
else()
  find_path(OGG_INCLUDE_DIRS ogg/ogg.h)
  find_library(OGG_LIBRARIES ogg)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OGG DEFAULT_MSG OGG_INCLUDE_DIRS OGG_LIBRARIES)

mark_as_advanced(OGG_INCLUDE_DIRS OGG_LIBRARIES)
