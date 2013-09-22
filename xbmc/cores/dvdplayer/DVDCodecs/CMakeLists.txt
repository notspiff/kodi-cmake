set(SOURCES DVDCodecUtils.cpp
            DVDFactoryCodec.cpp)

xbmc_add_library(dvdcodecs)
add_dependencies(dvdcodecs ffmpeg)

add_subdirectory(Audio)
add_subdirectory(Overlay)
add_subdirectory(Video)
