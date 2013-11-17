set(SOURCES ADPCMCodec.cpp
            ASAPCodec.cpp
            AudioDecoder.cpp
            CodecFactory.cpp
            DVDPlayerCodec.cpp
            MP3codec.cpp
            OggCallback.cpp
            OGGcodec.cpp
            PAPlayer.cpp
            PCMCodec.cpp
            SPCCodec.cpp
            TimidityCodec.cpp
            VGMCodec.cpp
            WAVcodec.cpp)

core_add_library(paplayer)
add_dependencies(paplayer ffmpeg)
