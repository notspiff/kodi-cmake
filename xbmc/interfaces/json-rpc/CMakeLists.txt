set(SOURCES AddonsOperations.cpp
            ApplicationOperations.cpp
            AudioLibrary.cpp
            FavouritesOperations.cpp
            FileItemHandler.cpp
            FileOperations.cpp
            GUIOperations.cpp
            InputOperations.cpp
            JSONRPC.cpp
            JSONServiceDescription.cpp
            PlayerOperations.cpp
            PlaylistOperations.cpp
            ProfilesOperations.cpp
            PVROperations.cpp
            SystemOperations.cpp
            VideoLibrary.cpp
            XBMCOperations.cpp)
  
xbmc_add_library(jsonrpc_interface)
add_dependencies(jsonrpc_interface libcpluff)
