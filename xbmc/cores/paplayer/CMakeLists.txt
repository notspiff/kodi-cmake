set(SOURCES ADPCMCodec.cpp
            ASAPCodec.cpp
            AudioDecoder.cpp
            CodecFactory.cpp
            DVDPlayerCodec.cpp
            FLACcodec.cpp
            ModplugCodec.cpp
            MP3codec.cpp
            NSFCodec.cpp
            OggCallback.cpp
            OGGcodec.cpp
            PAPlayer.cpp
            PCMCodec.cpp
            SIDCodec.cpp
            SPCCodec.cpp
            TimidityCodec.cpp
            VGMCodec.cpp
            WAVcodec.cpp
            YMCodec.cpp)

xbmc_add_library(paplayer)
add_dependencies(paplayer ffmpeg)
