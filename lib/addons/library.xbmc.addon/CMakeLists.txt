set(SOURCES libXBMC_addon.cpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${XBMC_SOURCE_DIR}/xbmc/addons/include
                    ${XBMC_SOURCE_DIR}/xbmc)

add_options(CXX ALL_BUILDS "-fPIC")

xbmc_add_library(XBMC_addon 1)
