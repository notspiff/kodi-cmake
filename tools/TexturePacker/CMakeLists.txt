set(SOURCES md5.cpp
            SDL_anigif.cpp
            XBTFWriter.cpp
            XBMCTex.cpp
            ${CORE_SOURCE_DIR}/xbmc/guilib/XBTF.cpp)

add_options(CXX ALL_BUILDS "-fPIC")

add_definitions(-DTARGET_POSIX -DUSE_LZO_PACKING)

find_package(SDL_image REQUIRED)

include_directories(${SDLIMAGE_INCLUDE_DIR})
add_executable(TexturePacker ${SOURCES})
target_link_libraries(TexturePacker squish ${SDLIMAGE_LIBRARY} ${SDL_LIBRARIES}
                                           ${LZO2_LIBRARIES})
