set(SOURCES PVRChannel.cpp
            PVRChannelGroup.cpp
            PVRChannelGroupInternal.cpp
            PVRChannelGroups.cpp
            PVRChannelGroupsContainer.cpp)

xbmc_add_library(pvr_channels)
add_dependencies(pvr_channels libcpluff)
