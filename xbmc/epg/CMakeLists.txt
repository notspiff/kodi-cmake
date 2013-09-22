set(SOURCES EpgContainer.cpp
            Epg.cpp
            EpgDatabase.cpp
            EpgInfoTag.cpp
            EpgSearchFilter.cpp
            GUIEPGGridContainer.cpp)

xbmc_add_library(epg)
add_dependencies(epg libcpluff)
