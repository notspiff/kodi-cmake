set(SOURCES DVDAudio.cpp
            DVDClock.cpp
            DVDDemuxSPU.cpp
            DVDFileInfo.cpp
            DVDMessage.cpp
            DVDMessageQueue.cpp
            DVDMessageTracker.cpp
            DVDOverlayContainer.cpp
            DVDOverlayRenderer.cpp
            DVDPerformanceCounter.cpp
            DVDPlayerAudio.cpp
            DVDPlayer.cpp
            DVDPlayerSubtitle.cpp
            DVDPlayerTeletext.cpp
            DVDPlayerVideo.cpp
            DVDStreamInfo.cpp
            DVDTSCorrection.cpp
            Edl.cpp)

xbmc_add_library(dvdplayer)
add_dependencies(dvdplayer libcpluff ffmpeg dvdnav)

add_subdirectory(DVDCodecs)
add_subdirectory(DVDDemuxers)
add_subdirectory(DVDInputStreams)
add_subdirectory(DVDSubtitles)
