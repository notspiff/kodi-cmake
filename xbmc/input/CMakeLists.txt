set(SOURCES ButtonTranslator.cpp
            InertialScrollingHandler.cpp
            KeyboardLayoutConfiguration.cpp
            KeyboardStat.cpp
            MouseStat.cpp
            XBMC_keytable.cpp)

if(SDL_FOUND)
  list(APPEND SOURCES SDLJoystick.cpp)
endif()
  
core_add_library(input)
