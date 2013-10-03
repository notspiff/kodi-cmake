project(zlib)

cmake_minimum_required(VERSION 2.8)

set(SOURCES adler32.c
            compress.c
            crc32.c
            deflate.c
            gzclose.c
            gzlib.c
            gzread.c
            gzwrite.c
            infback.c
            contrib/masmx64/inffas8664.c
            inffast.c
            inflate.c
            inftrees.c
            contrib/minizip/ioapi.c
            contrib/minizip/iowin32.c
            contrib/minizip/unzip.c
            contrib/minizip/zip.c
            zutil.c
            trees.c
            uncompr.c)

add_definitions(-DWIN32 -D_CRT_NONSTD_NO_DEPRECATE
		-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTD_NO_WARNINGS
		-DZLIB_WINAPI)

add_library(zlib ${SOURCES})

include_directories(${PROJECT_SOURCE_DIR})

set(HEADERS ${PROJECT_SOURCE_DIR}/zlib.h ${PROJECT_SOURCE_DIR}/zconf.h)

install(FILES ${HEADERS} DESTINATION ${OUTPUT_DIR}/include)
install(TARGETS zlib DESTINATION ${OUTPUT_DIR}/lib)
