set(SOURCES daap.c
            debug.c
            dmap_generics.c
            global.c
            httpClient.c
            ioloop.c
            libXDAAP.c
            threadpool.c
            Authentication/hasher.c
            Authentication/md5.c)

core_add_library(xdaap)
