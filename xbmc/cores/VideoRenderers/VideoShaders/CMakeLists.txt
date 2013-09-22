set(SOURCES ConvolutionKernels.cpp)

if(WIN32)
  list(APPEND SOURCES WinVideoFilter.cpp)
endif()

if(OPENGL_FOUND OR GLES_FOUND)
  list(APPEND SOURCES VideoFilterShader.cpp
                      YUV2RGBShader.cpp)
endif()

xbmc_add_library(videoshaders)
