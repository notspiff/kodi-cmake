set(SOURCES GUIDialogPictureInfo.cpp
            GUIViewStatePictures.cpp
            GUIWindowPictures.cpp
            GUIWindowSlideShow.cpp
            Picture.cpp
            PictureInfoLoader.cpp
            PictureInfoTag.cpp
            PictureThumbLoader.cpp
            SlideShowPicture.cpp)

xbmc_add_library(pictures)
add_dependencies(pictures ffmpeg)
