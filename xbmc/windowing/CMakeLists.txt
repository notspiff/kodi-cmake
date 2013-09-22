set(SOURCES WinSystem.cpp)

if(SDL_FOUND)
  list(APPEND SOURCES WinEventsSDL.cpp)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  list(APPEND SOURCES WinEventsLinux.cpp)
endif()
  
xbmc_add_library(windowing)

if (X_FOUND)
  add_subdirectory(X11)
endif()
