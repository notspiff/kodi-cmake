set(SOURCES Depacker.cpp
            digidrum.cpp
            LZH/lzhxlib.c
            XBMCYM.cpp
            Ym2149Ex.cpp
            Ymload.cpp
            YmMusic.cpp
            YmUserInterface.cpp)

add_options(ALL_LANGUAGES ALL_BUILDS "-fPIC")

add_definitions(-D_LINUX)

core_add_library(stsoundlibrary 1)
