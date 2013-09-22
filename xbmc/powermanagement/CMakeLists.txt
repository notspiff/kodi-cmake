set(SOURCES DPMSSupport.cpp
            PowerManager.cpp)

xbmc_add_library(powermanagement)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  add_subdirectory(linux)
endif()
