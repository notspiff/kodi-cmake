set(SOURCES LinuxInputDevices.cpp
            LIRC.cpp)

if(ENABLE_SDLJOYSTICK AND SDL_FOUND)
  list(APPEND SOURCES LinuxJoystickSDL.cpp)
endif()

if(LINUXJOYSTICK_FOUND)
  list(APPEND SOURCES LinuxJoystick.cpp)
endif()

core_add_library(input_linux)
