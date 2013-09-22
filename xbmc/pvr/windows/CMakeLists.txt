set(SOURCES GUIViewStatePVR.cpp
            GUIWindowPVR.cpp
            GUIWindowPVRChannels.cpp
            GUIWindowPVRCommon.cpp
            GUIWindowPVRGuide.cpp
            GUIWindowPVRRecordings.cpp
            GUIWindowPVRSearch.cpp
            GUIWindowPVRTimers.cpp)

xbmc_add_library(pvr_windows)
add_dependencies(pvr_windows libcpluff)
