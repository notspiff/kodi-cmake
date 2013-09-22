set(SOURCES libXBMC_codec.cpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${XBMC_SOURCE_DIR}/xbmc/addons/include)

add_library(XBMC_codec SHARED ${SOURCES})
