set(SOURCES WinSystemX11.cpp
            XRandR.cpp)

if(OPENGL_FOUND)
  list(APPEND SOURCES WinSystemX11GL.cpp)
endif()

xbmc_add_library(windowing_X11)
