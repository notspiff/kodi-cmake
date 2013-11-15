set(SOURCES AddonCallbacksAddon.cpp
            AddonCallbacksCodec.cpp
            AddonCallbacks.cpp
            AddonCallbacksGUI.cpp
            AddonCallbacksPVR.cpp
            Addon.cpp
            AddonDatabase.cpp
            AddonInstaller.cpp
            AddonManager.cpp
            AddonStatusHandler.cpp
            AddonVersion.cpp
            AudioEncoder.cpp
            GUIDialogAddonInfo.cpp
            GUIDialogAddonSettings.cpp
            GUIViewStateAddonBrowser.cpp
            GUIWindowAddonBrowser.cpp
            PluginSource.cpp
            Repository.cpp
            Scraper.cpp
            ScreenSaver.cpp
            Service.cpp
            Skin.cpp
            Visualisation.cpp)

core_add_library(addons)
add_dependencies(addons libcpluff ffmpeg)
