set(SOURCES libXBMC_pvr.cpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CORE_SOURCE_DIR}/xbmc/addons/include
                    ${CORE_SOURCE_DIR}/xbmc
                    ${CORE_SOURCE_DIR}/xbmc/cores/dvdplayer/DVDDemuxers)

add_options(CXX ALL_BUILDS "-fPIC")

core_add_library(XBMC_pvr 1)
