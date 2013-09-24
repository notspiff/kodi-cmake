set(SOURCES DPMSSupport.cpp
            PowerManager.cpp)

core_add_library(powermanagement)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  add_subdirectory(linux)
endif()
