set(SOURCES AudioDecoder.cpp
            CodecFactory.cpp
            DVDPlayerCodec.cpp
            PAPlayer.cpp
            PCMCodec.cpp)

core_add_library(paplayer)
add_dependencies(paplayer ffmpeg)
