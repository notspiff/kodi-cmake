# Base Io build system
# Written by Jeremy Tregunna <jeremy.tregunna@me.com>
#
# Find libyajl
 
find_path(YAJL_INCLUDE_DIRS yajl/yajl_common.h)
find_library(YAJL_LIBRARIES NAMES yajl)
 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(YAJL DEFAULT_MSG YAJL_INCLUDE_DIRS YAJL_LIBRARIES)
