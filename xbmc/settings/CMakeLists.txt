set(SOURCES AdvancedSettings.cpp
            DisplaySettings.cpp
            MediaSettings.cpp
            MediaSourceSettings.cpp
            SettingAddon.cpp
            SettingControl.cpp
            SettingPath.cpp
            Settings.cpp
            SkinSettings.cpp
            VideoSettings.cpp)

core_add_library(settings)
add_dependencies(settings libcpluff)
