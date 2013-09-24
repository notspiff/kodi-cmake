set(SOURCES PeripheralBus.cpp)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  if(LibUSB_FOUND OR UDEV_FOUND)
    add_subdirectory(linux)
  endif()
endif()

core_add_library(peripheral_bus)
