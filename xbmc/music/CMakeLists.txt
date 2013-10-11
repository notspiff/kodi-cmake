set(SOURCES Album.cpp
            Artist.cpp
            GUIViewStateMusic.cpp
            MusicDatabase.cpp
            MusicDbUrl.cpp
            MusicInfoLoader.cpp
            MusicThumbLoader.cpp
            Song.cpp)

core_add_library(music)
add_dependencies(music libcpluff)
