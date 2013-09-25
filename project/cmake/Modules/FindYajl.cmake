# Base Io build system
# Written by Jeremy Tregunna <jeremy.tregunna@me.com>
#
# Find libyajl
 

pkg_check_modules(YAJL yajl>=2.0)
if(YAJL_FOUND)
  list(APPEND YAJL_DEFINITIONS -DYAJL_MAJOR=2)
else()
  find_path(YAJL_INCLUDE_DIRS yajl/yajl_common.h)
  find_library(YAJL_LIBRARIES NAMES yajl)
endif()
 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Yajl DEFAULT_MSG YAJL_INCLUDE_DIRS YAJL_LIBRARIES)

#if(YAJL_FOUND)
#  find_path(YAJL_VERSION_INCLUDE yajl/yajl_version.h)
#  if(YAJL_VERSION_INCLUDE)
#    list(APPEND YAJL_DEFINITIONS -DYAJL_MAJOR=1)
#  endif()
#endif()
#mark_as_advanced(YAJL_INCLUDE_DIRS YAJL_LIBRARIES YAJL_DEFINITIONS)

