set(SOURCES PVRTimerInfoTag.cpp
            PVRTimers.cpp)

core_add_library(pvr_timers)
add_dependencies(pvr_timers libcpluff)
