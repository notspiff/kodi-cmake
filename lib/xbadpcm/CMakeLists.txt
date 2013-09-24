set(SOURCES ADPCMDll.cpp
            uXboxAdpcmDecoder.c)

add_options(ALL_LANGUAGES ALL_BUILDS "-fPIC")

add_definitions(-D_LINUX)

core_add_library(xbadpcm 1)
