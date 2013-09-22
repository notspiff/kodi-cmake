set(SOURCES PVRClient.cpp
            PVRClients.cpp)

xbmc_add_library(pvr_addons)
add_dependencies(pvr_addons libcpluff)
