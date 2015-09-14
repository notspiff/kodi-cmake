set(SOURCES ButtonTranslator.cpp
            InertialScrollingHandler.cpp
            InputManager.cpp
            InputCodingTableBaiduPY.cpp
            InputCodingTableBasePY.cpp
            InputCodingTableFactory.cpp
            Key.cpp
            KeyboardLayout.cpp
            KeyboardLayoutManager.cpp
            KeyboardStat.cpp
            MouseStat.cpp
            XBMC_keytable.cpp)

if(SDL_FOUND)
  list(APPEND SOURCES SDLJoystick.cpp)
endif()

core_add_library(input)
