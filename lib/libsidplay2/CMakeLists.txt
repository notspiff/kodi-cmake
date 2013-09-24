set(SOURCES builders/resid-builder/src/resid-builder.cpp
            builders/resid-builder/src/resid.cpp
            resid/sid.cc
            libsidplay/src/config.cpp
            libsidplay/src/event.cpp
            libsidplay/src/mixer.cpp
            libsidplay/src/player.cpp
            libsidplay/src/psiddrv.cpp
            libsidplay/src/reloc65.c
            libsidplay/src/sidplay2.cpp
            libsidplay/src/mos6510/mos6510.cpp
            libsidplay/src/mos6526/mos6526.cpp
            libsidplay/src/sid6526/sid6526.cpp
            libsidplay/src/mos656x/mos656x.cpp
            libsidplay/src/sidtune/IconInfo.cpp
            libsidplay/src/sidtune/InfoFile.cpp
            libsidplay/src/sidtune/MUS.cpp
            libsidplay/src/sidtune/p00.cpp
            libsidplay/src/sidtune/PP20.cpp
            libsidplay/src/sidtune/prg.cpp
            libsidplay/src/sidtune/PSID.cpp
            libsidplay/src/sidtune/SidTune.cpp
            libsidplay/src/sidtune/SidTuneTools.cpp
            libsidplay/src/xsid/xsid.cpp
            libsidplay/win/VC/XBMCSID.cpp
            resid/envelope.cc
            resid/extfilt.cc
            resid/filter.cc
            resid/pot.cc
            resid/version.cc
            resid/voice.cc
            resid/wave.cc
            resid/wave6581__ST.cc
            resid/wave6581_P_T.cc
            resid/wave6581_PS_.cc
            resid/wave6581_PST.cc
            resid/wave8580__ST.cc
            resid/wave8580_P_T.cc
            resid/wave8580_PS_.cc
            resid/wave8580_PST.cc)

add_options(ALL_LANGUAGES ALL_BUILDS "-fPIC")

add_definitions(-D_LINUX -DHAVE_UNIX -DHAVE_STRCASECMP
                -DHAVE_STRNCASECMP -DHAVE_IOS_OPENMODE
                -DPACKAGE_NAME="libsidplay2" -DPACKAGE_VERSION="1.0.0")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    libsidplay/win/VC
                    builders/resid-builder/include/sidplay/builders
                    libsidplay/include/sidplay
                    resid
                    libsidplay/include)

core_add_library(sidplay2 1)
