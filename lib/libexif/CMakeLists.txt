set(SOURCES ExifParse.cpp
            IptcParse.cpp
            JpegParse.cpp
            libexif.cpp)

add_options(CXX ALL_BUILDS "-fPIC")

add_definitions(-D_LINUX -D_DLL)

core_add_library(exif 1)
