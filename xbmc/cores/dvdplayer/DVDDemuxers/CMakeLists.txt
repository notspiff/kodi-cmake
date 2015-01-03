set(SOURCES DVDDemuxBXA.cpp
            DVDDemuxCC.cpp
            DVDDemuxCDDA.cpp
            DVDDemux.cpp
            DVDDemuxFFmpeg.cpp
            DVDDemuxHTSP.cpp
            DVDDemuxPVRClient.cpp
            DVDDemuxShoutcast.cpp
            DVDDemuxUtils.cpp
            DVDDemuxVobsub.cpp
            DVDFactoryDemuxer.cpp)

core_add_library(dvddemuxers)
add_dependencies(dvddemuxers libcpluff ffmpeg dvdnav)
