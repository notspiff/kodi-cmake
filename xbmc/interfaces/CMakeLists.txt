set(SOURCES AnnouncementManager.cpp
            Builtins.cpp)
  
xbmc_add_library(interfaces)
add_dependencies(interfaces libcpluff)

add_subdirectory(generic)
add_subdirectory(info)
add_subdirectory(json-rpc)
add_subdirectory(legacy)
add_subdirectory(python)
