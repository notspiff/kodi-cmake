set(SOURCES ExifParse.cpp
            GUIDialogPictureInfo.cpp
            GUIViewStatePictures.cpp
            GUIWindowPictures.cpp
            GUIWindowSlideShow.cpp
            IptcParse.cpp
            JpegParse.cpp
            libexif.cpp
            Picture.cpp
            PictureInfoLoader.cpp
            PictureInfoTag.cpp
            PictureThumbLoader.cpp
            SlideShowPicture.cpp)

core_add_library(pictures)
add_dependencies(pictures ffmpeg)
