set(SOURCES AddonBuiltins.cpp
            AndroidBuiltins.cpp
            ApplicationBuiltins.cpp
            Builtins.cpp
            CECBuiltins.cpp
            GUIBuiltins.cpp
            GUIContainerBuiltins.cpp
            GUIControlBuiltins.cpp
            LibraryBuiltins.cpp
            OpticalBuiltins.cpp
            PictureBuiltins.cpp
            PlayerBuiltins.cpp
            ProfileBuiltins.cpp
            PVRBuiltins.cpp
            SkinBuiltins.cpp
            SystemBuiltins.cpp
            WeatherBuiltins.cpp)

core_add_library(builtins)
add_dependencies(builtins libcpluff)
