set(SOURCES MusicAlbumInfo.cpp
            MusicArtistInfo.cpp
            MusicInfoScanner.cpp
            MusicInfoScraper.cpp)

xbmc_add_library(music_infoscanner)
add_dependencies(music_infoscanner libcpluff)
