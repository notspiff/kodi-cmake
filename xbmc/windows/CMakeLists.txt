set(SOURCES GUIMediaWindow.cpp
            GUIWindowDebugInfo.cpp
            GUIWindowFileManager.cpp
            GUIWindowHome.cpp
            GUIWindowLoginScreen.cpp
            GUIWindowPointer.cpp
            GUIWindowScreensaver.cpp
            GUIWindowScreensaverDim.cpp
            GUIWindowStartup.cpp
            GUIWindowSystemInfo.cpp
            GUIWindowWeather.cpp)

xbmc_add_library(windows)
add_dependencies(windows libcpluff)
