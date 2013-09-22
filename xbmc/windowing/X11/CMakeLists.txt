set(SOURCES WinSystemX11.cpp
            XRandR.cpp)

if(OPENGL_FOUND)
  list(APPEND SOURCES WinSystemX11GL.cpp)
endif()

core_add_library(windowing_X11)
