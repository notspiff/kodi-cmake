set(SOURCES AdvancedSettings.cpp
            DisplaySettings.cpp
            ISetting.cpp
            MediaSettings.cpp
            MediaSourceSettings.cpp
            Setting.cpp
            SettingAddon.cpp
            SettingCategoryAccess.cpp
            SettingConditions.cpp
            SettingControl.cpp
            SettingDependency.cpp
            SettingPath.cpp
            SettingSection.cpp
            Settings.cpp
            SettingsManager.cpp
            SettingRequirement.cpp
            SettingUpdate.cpp
            SkinSettings.cpp
            VideoSettings.cpp)

xbmc_add_library(settings)
add_dependencies(settings libcpluff)

add_subdirectory(dialogs)
add_subdirectory(windows)
