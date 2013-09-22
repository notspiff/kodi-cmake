set(SOURCES Atomics.cpp
            Event.cpp
            LockFree.cpp
            Thread.cpp
            Timer.cpp
            SystemClock.cpp
            platform/Implementation.cpp)

xbmc_add_library(threads)
