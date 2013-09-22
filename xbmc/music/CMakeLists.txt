set(SOURCES Album.cpp
            Artist.cpp
            GUIViewStateMusic.cpp
            MusicDatabase.cpp
            MusicDbUrl.cpp
            MusicInfoLoader.cpp
            MusicThumbLoader.cpp
            Song.cpp)

xbmc_add_library(music)
add_dependencies(music libcpluff)

add_subdirectory(dialogs)
add_subdirectory(infoscanner)
add_subdirectory(karaoke)
add_subdirectory(tags)
add_subdirectory(windows)
