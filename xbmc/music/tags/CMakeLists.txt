set(SOURCES MusicInfoTag.cpp
            MusicInfoTagLoaderASAP.cpp
            MusicInfoTagLoaderCDDA.cpp
            MusicInfoTagLoaderDatabase.cpp
            MusicInfoTagLoaderFactory.cpp
            MusicInfoTagLoaderMidi.cpp
            MusicInfoTagLoaderNSF.cpp
            MusicInfoTagLoaderShn.cpp
            MusicInfoTagLoaderSPC.cpp
            MusicInfoTagLoaderWav.cpp
            MusicInfoTagLoaderYM.cpp
            TagLibVFSStream.cpp
            TagLoaderTagLib.cpp)

core_add_library(music_tags)
