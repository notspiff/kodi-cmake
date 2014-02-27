find_path(LINUXJOYSTICK_INCLUDE_DIR linux/joystick.h)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LinuxJoystick DEFAULT_MSG LINUXJOYSTICK_INCLUDE_DIR)

if(LINUXJOYSTICK_FOUND)
  list(APPEND LINUXJOYSTICK_DEFINITIONS -DHAS_LINUX_JOYSTICK=1)
endif()
