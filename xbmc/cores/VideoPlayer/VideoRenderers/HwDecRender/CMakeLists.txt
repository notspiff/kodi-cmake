set(SOURCES)

if(VDPAU_FOUND)
  list(APPEND SOURCES RendererVDPAU.cpp)
endif()

if(VAAPI_FOUND)
  list(APPEND SOURCES RendererVAAPI.cpp)
endif()

core_add_library(hwdecrender)
add_dependencies(hwdecrender ffmpeg)
