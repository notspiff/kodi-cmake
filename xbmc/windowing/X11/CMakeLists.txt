if(X_FOUND)
  list(APPEND SOURCES XRandR.cpp
                      WinSystemX11.cpp)

  if(OPENGL_FOUND)
    list(APPEND SOURCES GLContextGLX.cpp
                        GLContextEGL.cpp
                        WinSystemX11GLContext.cpp)
  endif()
  core_add_library(windowing_X11)
endif()
