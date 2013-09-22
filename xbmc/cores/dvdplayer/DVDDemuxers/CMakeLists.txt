set(SOURCES DVDDemuxBXA.cpp
            DVDDemuxCDDA.cpp
            DVDDemux.cpp
            DVDDemuxFFmpeg.cpp
            DVDDemuxHTSP.cpp
            DVDDemuxPVRClient.cpp
            DVDDemuxShoutcast.cpp
            DVDDemuxUtils.cpp
            DVDDemuxVobsub.cpp
            DVDFactoryDemuxer.cpp)

xbmc_add_library(dvddemuxers)
add_dependencies(dvddemuxers libcpluff ffmpeg)
