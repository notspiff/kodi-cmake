set(SOURCES ConvUtils.cpp
            DBusMessage.cpp
            DBusReserve.cpp
            DBusUtil.cpp
            HALManager.cpp
            LinuxResourceCounter.cpp
            LinuxTimezone.cpp
            PosixMountProvider.cpp
            RBP.cpp
            XFileUtils.cpp
            XHandle.cpp
            XMemUtils.cpp
            XTimeUtils.cpp)

if(OMXLIB_FOUND)
  list(APPEND SOURCES OMXClock.cpp
                      OMXCore.cpp)
endif()
  
xbmc_add_library(linuxsupport)
