set(SOURCES libXBMC_pvr.cpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${XBMC_SOURCE_DIR}/xbmc/addons/include
                    ${XBMC_SOURCE_DIR}/xbmc
                    ${XBMC_SOURCE_DIR}/xbmc/cores/dvdplayer/DVDDemuxers)

add_library(XBMC_pvr SHARED ${SOURCES})
