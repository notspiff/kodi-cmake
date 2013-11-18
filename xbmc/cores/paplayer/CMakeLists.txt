set(SOURCES AudioDecoder.cpp
            CodecFactory.cpp
            DVDPlayerCodec.cpp
            MP3codec.cpp
            PAPlayer.cpp
            PCMCodec.cpp
            WAVcodec.cpp)

core_add_library(paplayer)
add_dependencies(paplayer ffmpeg)
