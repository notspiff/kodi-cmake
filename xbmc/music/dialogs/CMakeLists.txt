set(SOURCES GUIDialogMusicInfo.cpp
            GUIDialogMusicOSD.cpp
            GUIDialogMusicOverlay.cpp
            GUIDialogSongInfo.cpp
            GUIDialogVisualisationPresetList.cpp)

xbmc_add_library(music_dialogs)
add_dependencies(music_dialogs libcpluff)
