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
            AudioDecoder.cpp
            AudioEncoder.cpp
            ContextItemAddon.cpp
            GUIDialogAddonInfo.cpp
            GUIDialogAddonSettings.cpp
            GUIViewStateAddonBrowser.cpp
            GUIWindowAddonBrowser.cpp
            LanguageResource.cpp
            PluginSource.cpp
            Repository.cpp
            Scraper.cpp
            ScreenSaver.cpp
            Service.cpp
            Skin.cpp
            UISoundsResource.cpp
            VFSEntry.cpp
            Visualisation.cpp
            Webinterface.cpp)

core_add_library(addons)
add_dependencies(addons libcpluff ffmpeg)
