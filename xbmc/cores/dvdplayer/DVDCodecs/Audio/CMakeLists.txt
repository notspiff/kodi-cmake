set(SOURCES DVDAudioCodecFFmpeg.cpp
            DVDAudioCodecLPcm.cpp
            DVDAudioCodecPassthrough.cpp
            DVDAudioCodecPcm.cpp)

core_add_library(dvdaudiocodecs)
add_dependencies(dvdaudiocodecs ffmpeg)
