# - Try to find libtag
# Once done this will define
#
# TAGLIB_FOUND - system has libtag
# TAGLIB_INCLUDE_DIRS - the libtag include directory
# TAGLIB_LIBRARIES - The libtag libraries

if(PKG_CONFIG_FOUND)
  pkg_check_modules (TAGLIB taglib>=1.8.0)
else()
  find_path(TAGLIB_INCLUDE_DIRS taglib/tag.h)
  find_library(TAGLIB_LIBRARIES tag)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TAGLIB DEFAULT_MSG TAGLIB_INCLUDE_DIRS TAGLIB_LIBRARIES)

mark_as_advanced(TAGLIB_INCLUDE_DIRS TAGLIB_LIBRARIES)
