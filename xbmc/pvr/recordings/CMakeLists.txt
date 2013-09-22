set(SOURCES PVRRecording.cpp
            PVRRecordings.cpp)

xbmc_add_library(pvr_recordings)
add_dependencies(pvr_recordings libcpluff)
