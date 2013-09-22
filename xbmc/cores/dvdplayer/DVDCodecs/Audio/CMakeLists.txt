set(SOURCES DVDAudioCodecFFmpeg.cpp
            DVDAudioCodecLibMad.cpp
            DVDAudioCodecLPcm.cpp
            DVDAudioCodecPassthrough.cpp
            DVDAudioCodecPassthroughFFmpeg.cpp
            DVDAudioCodecPcm.cpp)

xbmc_add_library(dvdaudiocodecs)
add_dependencies(dvdaudiocodecs ffmpeg)
