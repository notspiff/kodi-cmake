set(SOURCES UPnP.cpp
            UPnPInternal.cpp
            UPnPPlayer.cpp
            UPnPRenderer.cpp
            UPnPServer.cpp
            UPnPSettings.cpp)

INCLUDE_DIRECTORIES(${XBMC_SOURCE_DIR}/lib/libUPnP/Platinum/Source/Core
                    ${XBMC_SOURCE_DIR}/lib/libUPnP/Platinum/Source/Platinum
                    ${XBMC_SOURCE_DIR}/lib/libUPnP/Platinum/Source/Devices/MediaConnect
                    ${XBMC_SOURCE_DIR}/lib/libUPnP/Platinum/Source/Devices/MediaRenderer
                    ${XBMC_SOURCE_DIR}/lib/libUPnP/Platinum/Source/Devices/MediaServer
                    ${XBMC_SOURCE_DIR}/lib/libUPnP/Platinum/Source/Extras
                    ${XBMC_SOURCE_DIR}/lib/libUPnP/Neptune/Source/System/Posix
                    ${XBMC_SOURCE_DIR}/lib/libUPnP/Neptune/Source/Core)
  
xbmc_add_library(network_upnp)
