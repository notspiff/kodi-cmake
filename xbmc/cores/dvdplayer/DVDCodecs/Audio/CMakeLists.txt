set(SOURCES DVDAudioCodecFFmpeg.cpp
            DVDAudioCodecLibMad.cpp
            DVDAudioCodecLPcm.cpp
            DVDAudioCodecPassthrough.cpp
            DVDAudioCodecPassthroughFFmpeg.cpp
            DVDAudioCodecPcm.cpp)

core_add_library(dvdaudiocodecs)
add_dependencies(dvdaudiocodecs ffmpeg)
