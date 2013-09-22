# - Try to find CURL
# Once done this will define
#
# CURL_FOUND - system has libcurl
# CURL_INCLUDE_DIRS - the libcurl include directory
# CURL_LIBRARIES - The libcurl libraries

if(PKG_CONFIG_FOUND)
  pkg_check_modules (CURL libcurl)
  list(APPEND CURL_INCLUDE_DIRS ${CURL_INCLUDEDIR})
else()
  find_package(CURL)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CURL DEFAULT_MSG CURL_INCLUDE_DIRS CURL_LIBRARIES)

mark_as_advanced(CURL_INCLUDE_DIRS CURL_LIBRARIES)
