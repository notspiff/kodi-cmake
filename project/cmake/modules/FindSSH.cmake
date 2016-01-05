# - Try to find 
# Once done this will define
#
# SSH_FOUND - system has libSSH
# SSH_INCLUDE_DIRS - the libSSH include directory
# SSH_LIBRARIES - The libSSH libraries

if(PKG_CONFIG_FOUND)
  pkg_check_modules (SSH libssh)
endif()
set(SSH_INCLUDE_DIRS ${SSH_INCLUDEDIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SSH DEFAULT_MSG SSH_LIBRARIES)
list(APPEND SSH_DEFINITIONS -DHAVE_LIBSSH=1)

mark_as_advanced(SSH_INCLUDE_DIRS SSH_LIBRARIES)
