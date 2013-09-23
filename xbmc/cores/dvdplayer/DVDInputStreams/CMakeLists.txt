set(SOURCES DVDFactoryInputStream.cpp
            DVDInputStream.cpp
            DVDInputStreamFFmpeg.cpp
            DVDInputStreamFile.cpp
            DVDInputStreamHTSP.cpp
            DVDInputStreamHttp.cpp
            DVDInputStreamMemory.cpp
            DVDInputStreamNavigator.cpp
            DVDInputStreamPVRManager.cpp
            DVDInputStreamRTMP.cpp
            DVDInputStreamStack.cpp
            DVDInputStreamTV.cpp
            DVDStateSerializer.cpp)


if(BLURAY_FOUND)
  list(APPEND SOURCES DVDInputStreamBluray.cpp)
endif()

xbmc_add_library(dvdinputstreams)
add_dependencies(dvdinputstreams libcpluff dvdnav)
