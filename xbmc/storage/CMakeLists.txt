set(SOURCES AutorunMediaJob.cpp
            cdioSupport.cpp
            DetectDVDType.cpp
            IoSupport.cpp
            MediaManager.cpp)

xbmc_add_library(storage)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  add_subdirectory(linux)
endif()
