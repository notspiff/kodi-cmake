set(SOURCES coff.cpp
            dll.cpp
            DllLoader.cpp
            DllLoaderContainer.cpp
            dll_tracker.cpp
            dll_tracker_file.cpp
            dll_tracker_library.cpp
            dll_util.cpp
            ldt_keeper.c
            LibraryLoader.cpp
            mmap_anon.c
            SoLoader.cpp)

add_subdirectory(exports)

add_definitions(-DAPI_DEBUG)

xbmc_add_library(dllloader)
