set(SOURCES DVDOverlayCodecCC.cpp
            DVDOverlayCodec.cpp
            DVDOverlayCodecFFmpeg.cpp
            DVDOverlayCodecSSA.cpp
            DVDOverlayCodecText.cpp
            DVDOverlayCodecTX3G.cpp
            libspucc/cc_decoder.c)

core_add_library(dvdoverlaycodecs)
add_dependencies(dvdoverlaycodecs ffmpeg)
