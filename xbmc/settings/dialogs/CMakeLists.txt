set(SOURCES GUIDialogContentSettings.cpp
            GUIDialogSettings.cpp)

core_add_library(settings_dialogs)
add_dependencies(settings_dialogs libcpluff)
