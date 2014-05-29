set(SOURCES md5.cpp
            SDL_anigif.cpp
            XBTFWriter.cpp
            XBMCTex.cpp
            ${CORE_SOURCE_DIR}/xbmc/guilib/XBTF.cpp)

include(addoptions)
include(macros)
add_options(CXX ALL_BUILDS "-fPIC")

# Fixme
add_definitions(-DTARGET_POSIX -DUSE_LZO_PACKING)

list(APPEND CMAKE_MODULE_PATH ${CORE_SOURCE_DIR}/project/cmake/modules)

find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(Lzo2 REQUIRED)

set(SQUISH_TARGET squish-native)
add_subdirectory(${CORE_SOURCE_DIR}/lib/libsquish squish-native)
set(SQUISH_TARGET)

include_directories(${SDLIMAGE_INCLUDE_DIR}
                    ${SDL_INCLUDE_DIR}
                    ${CORE_SOURCE_DIR}/xbmc
                    ${CORE_SOURCE_DIR}/lib
                    ${CORE_SOURCE_DIR}/xbmc/linux) # Fixme
add_executable(TexturePacker ${SOURCES})
target_link_libraries(TexturePacker ${SYSTEM_LDFLAGS} squish-native
                                    ${SDLIMAGE_LIBRARY} ${SDL_LIBRARY}
                                    ${LZO2_LIBRARIES})
