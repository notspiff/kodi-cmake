set(SOURCES GLContextGLX.cpp
            XRandR.cpp)

if(OPENGL_FOUND)
  list(APPEND SOURCES WinSystemX11GLContext.cpp)
endif()

core_add_library(windowing_X11)
