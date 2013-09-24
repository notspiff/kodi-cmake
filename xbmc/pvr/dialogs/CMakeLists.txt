set(SOURCES GUIDialogPVRChannelManager.cpp
            GUIDialogPVRChannelsOSD.cpp
            GUIDialogPVRCutterOSD.cpp
            GUIDialogPVRDirectorOSD.cpp
            GUIDialogPVRGroupManager.cpp
            GUIDialogPVRGuideInfo.cpp
            GUIDialogPVRGuideOSD.cpp
            GUIDialogPVRGuideSearch.cpp
            GUIDialogPVRRecordingInfo.cpp
            GUIDialogPVRTimerSettings.cpp)

core_add_library(pvr_dialogs)
add_dependencies(pvr_dialogs libcpluff)
