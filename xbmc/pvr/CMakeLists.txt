set(SOURCES PVRGUIInfo.cpp
            PVRManager.cpp
            PVRDatabase.cpp)

core_add_library(pvr)
add_dependencies(pvr libcpluff)

add_subdirectory(addons)
add_subdirectory(channels)
add_subdirectory(dialogs)
add_subdirectory(recordings)
add_subdirectory(timers)
add_subdirectory(windows)
