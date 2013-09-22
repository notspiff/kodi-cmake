set(SOURCES RenderSystem.cpp)

xbmc_add_library(rendering)

if (OPENGL_FOUND)
  add_subdirectory(gl)
endif()
