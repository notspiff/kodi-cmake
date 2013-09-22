set(SOURCES libXBMC_gui.cpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${XBMC_SOURCE_DIR}/xbmc/addons/include
                    ${XBMC_SOURCE_DIR}/xbmc
                    ${XBMC_SOURCE_DIR}/xbmc/cores/dvdplayer/DVDDemuxers)

add_options(CXX ALL_BUILDS "-fPIC")

core_add_library(XBMC_gui 1)
