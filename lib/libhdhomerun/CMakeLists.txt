set(SOURCES hdhomerun_channels.c
            hdhomerun_channelscan.c
            hdhomerun_control.c
            hdhomerun_debug.c
            hdhomerun_device.c
            hdhomerun_discover.c
            hdhomerun_pkt.c
            hdhomerun_video.c)

add_options(C ALL_BUILDS "-Wall -Wmissing-declarations -Wmissing-prototypes -Wstrict-prototypes -Wpointer-arith -fPIC")

add_definitions(-D__MINGW__)

core_add_library(hdhomerun 1)
