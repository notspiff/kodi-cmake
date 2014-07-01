set(SOURCES DVDAudio.cpp
            DVDClock.cpp
            DVDDemuxSPU.cpp
            DVDFileInfo.cpp
            DVDMessage.cpp
            DVDMessageQueue.cpp
            DVDOverlayContainer.cpp
            DVDOverlayRenderer.cpp
            DVDPlayerAudio.cpp
            DVDPlayer.cpp
            DVDPlayerSubtitle.cpp
            DVDPlayerTeletext.cpp
            DVDPlayerVideo.cpp
            DVDStreamInfo.cpp
            DVDTSCorrection.cpp
            Edl.cpp)

core_add_library(dvdplayer)
add_dependencies(dvdplayer libcpluff ffmpeg dvdnav)
