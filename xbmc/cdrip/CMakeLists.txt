set(SOURCES CDDARipJob.cpp
            CDDARipper.cpp
            Encoder.cpp
            EncoderFFmpeg.cpp
            EncoderWav.cpp)

core_add_library(cdrip)
add_dependencies(cdrip ffmpeg)
