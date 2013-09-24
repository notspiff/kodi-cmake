SET(SOURCES libcmyth/bookmark.c
            libcmyth/commbreak.c
            libcmyth/connection.c
            libcmyth/debug.c
            libcmyth/event.c
            libcmyth/file.c
            libcmyth/freespace.c
            libcmyth/keyframe.c
            libcmyth/livetv.c
            libcmyth/mysql_query.c
            libcmyth/mythtv_mysql.c
            libcmyth/posmap.c
            libcmyth/proginfo.c
            libcmyth/proglist.c
            libcmyth/rec_num.c
            libcmyth/recorder.c
            libcmyth/ringbuf.c
            libcmyth/socket.c
            libcmyth/timestamp.c
            libcmyth/utf8tolatin1.c
            librefmem/alloc.c
            librefmem/debug_refmem.c)

include_directories(libcmyth librefmem include)

add_options(C ALL_BUILDS "-fPIC")

core_add_library(cmyth 1)
