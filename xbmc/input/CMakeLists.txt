set(SOURCES ButtonTranslator.cpp
            InertialScrollingHandler.cpp
            KeyboardLayoutConfiguration.cpp
            KeyboardStat.cpp
            MouseStat.cpp
            XBMC_keytable.cpp)

if(SDL_FOUND)
  list(APPEND SOURCES SDLJoystick.cpp)
endif()
  
xbmc_add_library(input)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  add_subdirectory(linux)
endif()
