set(SOURCES PlayerCoreFactory.cpp
            PlayerSelectionRule.cpp)

xbmc_add_library(playercorefactory)
add_dependencies(playercorefactory ffmpeg)
