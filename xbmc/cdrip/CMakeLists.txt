set(SOURCES CDDARipJob.cpp
            CDDARipper.cpp
            Encoder.cpp
            EncoderFFmpeg.cpp
            EncoderFlac.cpp
            EncoderVorbis.cpp
            EncoderWav.cpp)

if(LAME_FOUND)
  list(APPEND SOURCES EncoderLame.cpp)
endif()

core_add_library(cdrip)
add_dependencies(cdrip ffmpeg)
