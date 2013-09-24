set(SOURCES RenderSystem.cpp)

core_add_library(rendering)

if (OPENGL_FOUND)
  add_subdirectory(gl)
endif()
