set(SOURCES MusicInfoTag.cpp
            MusicInfoTagLoaderCDDA.cpp
            MusicInfoTagLoaderDatabase.cpp
            MusicInfoTagLoaderFactory.cpp
            MusicInfoTagLoaderShn.cpp
            MusicInfoTagLoaderWav.cpp
            TagLibVFSStream.cpp
            TagLoaderTagLib.cpp)

core_add_library(music_tags)

add_dependencies(music_tags libcpluff)
