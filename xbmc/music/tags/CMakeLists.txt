set(SOURCES MusicInfoTag.cpp
            MusicInfoTagLoaderASAP.cpp
            MusicInfoTagLoaderCDDA.cpp
            MusicInfoTagLoaderDatabase.cpp
            MusicInfoTagLoaderFactory.cpp
            MusicInfoTagLoaderMidi.cpp
            MusicInfoTagLoaderShn.cpp
            MusicInfoTagLoaderSPC.cpp
            MusicInfoTagLoaderWav.cpp
            TagLibVFSStream.cpp
            TagLoaderTagLib.cpp)

core_add_library(music_tags)

add_dependencies(music_tags libcpluff)
