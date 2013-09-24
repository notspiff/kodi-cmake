project(xbmc)

cmake_minimum_required(VERSION 2.8.8)

include(ExternalProject)
include(AddOptions.cmake)
include(Scripts/XbmcMacros.cmake)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/Modules)

enable_language(CXX)
enable_language(C)

option(ENABLE_DVDCSS      "Enable libdvdcss support?" ON)
option(ENABLE_PULSEAUDIO  "Enable PulseAudio support?" OFF)
option(ENABLE_VDPAU       "Enable VDPAU support?" ON)
option(ENABLE_VAAPI       "Enable VAAPI support?" ON)
option(ENABLE_UPNP        "Enable UPnP support?" ON)
option(ENABLE_NONFREE     "Enable non-free components?" ON)
option(ENABLE_MICROHTTPD  "Enable MicroHttpd webserver?" ON)
option(ENABLE_LAME        "Enable LAME mp3 encoder?" ON)
option(ENABLE_MYSQLCLIENT "Enable MySql support?" ON)
option(ENABLE_ALSA        "Enable ALSA support?" ON)
option(ENABLE_UDEV        "Enable UDev support?" ON)
option(ENABLE_LIBUSB      "Enable libusb support?" ON)
option(ENABLE_AVAHI       "Enable Avahi support?" ON)
option(ENABLE_RTMP        "Enable RTMP support?" ON)
option(ENABLE_BLURAY      "Enable BluRay support?" ON)
option(ENABLE_AFPCLIENT   "Enable AFP support?" ON)
option(ENABLE_PLIST       "Enable AirPlay support?" ON)
option(ENABLE_AIRTUNES    "Enable AirTunes support?" ON)
option(ENABLE_SSH         "Enable SSH support?" ON)
option(ENABLE_NFS         "Enable NFS support?" ON)
option(ENABLE_X11         "Enable X11 support?" ON)
option(ENABLE_SDL         "Enable SDL?" ON)
option(ENABLE_OPENGL      "Enable OpenGL?" ON)
option(ENABLE_EXTERNAL_FFMPEG "Enable external ffmpeg?" OFF)
option(ENABLE_EXTERNAL_LIBAV "Enable external libav?" OFF)
set(LIRC_DEVICE          "\"/dev/lircd\"" CACHE STRING "LIRC device to use")

set(XBMC_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../..)
set(XBMC_BUILD_DIR build)
if(BOOTSTRAP_DIR)
  set(UPDATE_COMMAND "")
else()
  set(UPDATE_COMMAND autoreconf -vif)
endif()

include(Scripts/XbmcArchSetup.cmake)

set(INCLUDES ${XBMC_SOURCE_DIR}/addons/library.xbmc.addon
             ${XBMC_SOURCE_DIR}/addons/library.xbmc.codec
             ${XBMC_SOURCE_DIR}/addons/library.xbmc.gui
             ${XBMC_SOURCE_DIR}/addons/library.xbmc.pvr
             ${XBMC_SOURCE_DIR}/lib
             ${XBMC_SOURCE_DIR}/xbmc
             ${XBMC_SOURCE_DIR}/xbmc/linux
             ${XBMC_SOURCE_DIR}/xbmc/cores/dvdplayer
             ${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR})

find_package(PkgConfig)
find_package(Threads REQUIRED)
list(APPEND DEPLIBS ${CMAKE_THREAD_LIBS_INIT})

# Required dependencies
set(required_deps Sqlite3 FreeType PCRE Cpluff
                  TinyXML Python Yajl LibSmbClient
                  JPEG Lzo2 Fribidi SampleRate TagLib FFMPEG)

# Optional dependencies
set(optional_deps MicroHttpd MySqlClient SSH
                  Alsa UDev Dbus Avahi
                  PulseAudio VDPAU VAAPI)

# Required, dyloaded deps
set(required_dyload Curl MAD Modplug ASS Mpeg2
                    Ogg Vorbis VorbisEnc VorbisFile LibDvd)

# Optional, dyloaded deps
set(dyload_optional Lame RTMP CEC Bluray
                    AfpClient Plist NFS)

# Required by shared objects we link
set(required_dep_libs Jasper PNG TIFF ZLIB EXPAT)

foreach(dep ${required_deps})
  xbmc_require_dep(${dep})
endforeach()

foreach(dep ${required_dep_libs})
  find_package(${dep} REQUIRED)
endforeach()

foreach(dep ${optional_deps})
  xbmc_optional_dep(${dep})
endforeach()

if(NOT UDEV_FOUND)
  xbmc_optional_dep(LibUsb)
endif()

foreach(dep ${required_dyload})
  xbmc_require_dyload_dep(${dep})
endforeach()

# Need a special case due to upper case lib name
find_package(FLAC REQUIRED)
xbmc_find_soname(FLAC FLAC)

foreach(dep ${dyload_optional})
  xbmc_optional_dyload_dep(${dep})
endforeach()

if(ENABLE_UPNP)
  list(APPEND DEP_DEFINES "-DUSE_UPNP=1")
endif()

if(ENABLE_NONFREE)
  list(APPEND DEP_DEFINES "-DHAVE_XBMC_NONFREE=1")
endif()

xbmc_optional_dep(Shairplay ENABLE_AIRTUNES)
if(NOT SHAIRPLAY_FOUND)
  xbmc_optional_dep(Shairport ENABLE_AIRTUNES)
endif()

if(NOT WIN32)
  xbmc_optional_dep(OpenGl)
  if(OPENGL_FOUND)
    xbmc_require_dep(GLEW)
    xbmc_optional_dep(X ENABLE_X11)
    xbmc_optional_dep(XRandR ENABLE_X11)
    xbmc_optional_dep(Sdl)
  endif()
endif()

if(ENABLE_SKIN_TOUCHED)
  list(APPEND DEP_DEFINES "-DHAS_SKIN_TOUCHED=1")
endif()

include_directories(${INCLUDES} ${SYSTEM_INCLUDES})
add_definitions(${ARCH_DEFINES} ${SYSTEM_DEFINES} ${DEP_DEFINES})

set(xbmc-bin_DEPENDS "" CACHE STRING "" FORCE)

if(NOT BOOTSTRAP_DIR)
  set(BOOTSTRAP_IN_TREE 1)
endif()
add_subdirectory(bootstrap ${XBMC_BUILD_DIR}/bootstrap.dir)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/linux                 ${XBMC_BUILD_DIR}/linuxsupport.dir)
endif()

add_subdirectory(${XBMC_SOURCE_DIR}/xbmc                         ${XBMC_BUILD_DIR}/xbmc.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/addons                  ${XBMC_BUILD_DIR}/addons.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/commons                 ${XBMC_BUILD_DIR}/commons.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/cores                   ${XBMC_BUILD_DIR}/cores.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/cores/AudioEngine       ${XBMC_BUILD_DIR}/audioengine.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/cores/DllLoader         ${XBMC_BUILD_DIR}/dllloader.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/cores/dvdplayer         ${XBMC_BUILD_DIR}/dvdplayer.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/cores/ExternalPlayer    ${XBMC_BUILD_DIR}/externalplayer.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/cores/paplayer          ${XBMC_BUILD_DIR}/paplayer.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/cores/playercorefactory ${XBMC_BUILD_DIR}/playercorefactory.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/cores/VideoRenderers    ${XBMC_BUILD_DIR}/videorenderers.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/cdrip                   ${XBMC_BUILD_DIR}/cdrip.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/dbwrappers              ${XBMC_BUILD_DIR}/dbwrappers.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/dialogs                 ${XBMC_BUILD_DIR}/dialogs.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/epg                     ${XBMC_BUILD_DIR}/epg.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/filesystem              ${XBMC_BUILD_DIR}/filesystem.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/guilib                  ${XBMC_BUILD_DIR}/guilib.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/input                   ${XBMC_BUILD_DIR}/input.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/interfaces              ${XBMC_BUILD_DIR}/interfaces.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/music                   ${XBMC_BUILD_DIR}/music.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/network                 ${XBMC_BUILD_DIR}/network.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/peripherals             ${XBMC_BUILD_DIR}/peripherals.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/pictures                ${XBMC_BUILD_DIR}/pictures.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/playlists               ${XBMC_BUILD_DIR}/playlists.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/powermanagement         ${XBMC_BUILD_DIR}/powermanagement.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/profiles                ${XBMC_BUILD_DIR}/profiles.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/programs                ${XBMC_BUILD_DIR}/programs.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/pvr                     ${XBMC_BUILD_DIR}/pvr.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/rendering               ${XBMC_BUILD_DIR}/rendering.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/settings                ${XBMC_BUILD_DIR}/settings.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/storage                 ${XBMC_BUILD_DIR}/storage.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/threads                 ${XBMC_BUILD_DIR}/threads.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/utils                   ${XBMC_BUILD_DIR}/utils.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/video                   ${XBMC_BUILD_DIR}/video.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/view                    ${XBMC_BUILD_DIR}/view.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/windowing               ${XBMC_BUILD_DIR}/windowing.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/windows                 ${XBMC_BUILD_DIR}/windows.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/tools/TexturePacker          ${XBMC_BUILD_DIR}/texturepacker.dir)

# platform specific
if(WIN32)
  add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/input/windows         ${XBMC_BUILD_DIR}/wininput.dir)
endif()

add_subdirectory(${XBMC_SOURCE_DIR}/lib/addons/library.xbmc.addon ${XBMC_BUILD_DIR}/XBMC_addon.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/addons/library.xbmc.codec ${XBMC_BUILD_DIR}/XBMC_codec.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/addons/library.xbmc.gui   ${XBMC_BUILD_DIR}/XBMC_gui.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/addons/library.xbmc.pvr   ${XBMC_BUILD_DIR}/XBMC_pvr.dir)

add_subdirectory(${XBMC_SOURCE_DIR}/lib/cmyth                    ${XBMC_BUILD_DIR}/cmyth.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/cximage-6.0              ${XBMC_BUILD_DIR}/cximage.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/libexif                  ${XBMC_BUILD_DIR}/exif.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/libhdhomerun             ${XBMC_BUILD_DIR}/hdhomerun.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/libhts                   ${XBMC_BUILD_DIR}/hts.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/libRTV                   ${XBMC_BUILD_DIR}/rtv.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/libsidplay2              ${XBMC_BUILD_DIR}/sidplay2.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/libsquish                ${XBMC_BUILD_DIR}/squish.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/libUPnP                  ${XBMC_BUILD_DIR}/upnp.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/libXDAAP                 ${XBMC_BUILD_DIR}/xdaap.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/nosefart                 ${XBMC_BUILD_DIR}/nosefart.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/SlingboxLib              ${XBMC_BUILD_DIR}/slingbox.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/stsound/StSoundLibrary   ${XBMC_BUILD_DIR}/stsoundlibrary.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/timidity                 ${XBMC_BUILD_DIR}/timidity.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/vgmstream                ${XBMC_BUILD_DIR}/vgmstream.dir)
add_subdirectory(${XBMC_SOURCE_DIR}/lib/xbadpcm                  ${XBMC_BUILD_DIR}/xbadpcm.dir)

if(ENABLE_EXTERNAL_FFMPEG OR ENABLE_EXTERNAL_LIBAV)
  add_subdirectory(${XBMC_SOURCE_DIR}/lib/xbmc-dll-symbols ${XBMC_BUILD_DIR}/dll_symbols.dir)
endif()
if(ENABLE_EXTERNAL_LIBAV)
  add_subdirectory(${XBMC_SOURCE_DIR}/lib/xbmc-libav-hacks ${XBMC_BUILD_DIR}/libav-hacks.dir)
endif()

# non-free libraries
if(ENABLE_NONFREE)
  add_subdirectory(${XBMC_SOURCE_DIR}/lib/UnrarXLib              ${XBMC_BUILD_DIR}/unrarxlib.dir)
endif()

# main binary
add_executable(xbmc.bin ${XBMC_SOURCE_DIR}/xbmc/main/main.cpp)
target_link_libraries(xbmc.bin -Wl,--whole-archive ${xbmc-bin_DEPENDS} -Wl,--no-whole-archive ${DEPLIBS})
install(TARGETS xbmc.bin DESTINATION lib/xbmc)

# copy files to build tree
set(copy_dirs addons/xbmc.addon/*
              addons/xbmc.codec/*
              addons/xbmc.core/*
              addons/xbmc.debug/*
              addons/xbmc.gui/*
              addons/xbmc.json/*
              addons/xbmc.metadata/*
              addons/xbmc.pvr/*
              addons/xbmc.python/*
              addons/library.xbmc.addon/*
              addons/library.xbmc.codec/*
              addons/library.xbmc.gui/*
              addons/library.xbmc.pvr/*
              addons/repository.xbmc.org/*
              addons/webinterface.default/*
              addons/screensaver.xbmc.builtin.dim/*
              addons/screensaver.xbmc.builtin.black/*
              language/*
              media/*
              sounds/*
              system/keymaps/*
              system/library/*
              system/players/dvdplayer/etc/*
              system/shaders/*
              system/settings/*
              userdata/*
              system/*.xml)

foreach(dir ${copy_dirs})
  file(GLOB_RECURSE FILES RELATIVE ${XBMC_SOURCE_DIR} ${XBMC_SOURCE_DIR}/${dir})
  foreach(file ${FILES})
    copy_file_to_buildtree(${XBMC_SOURCE_DIR}/${file} ${XBMC_SOURCE_DIR})
  endforeach()
endforeach()

list(APPEND SKINS "${XBMC_SOURCE_DIR}/addons/skin.confluence\;${XBMC_SOURCE_DIR}")

# These are skins that are copied into place from the source tree
foreach(skin ${SKINS})
  list(GET skin 0 dir)
  list(GET skin 1 relative)
  copy_skin_to_buildtree(${dir} ${relative})
endforeach()

# These for skins we cloned from a git within the build system
foreach(xbt ${EXTRA_XBT})
  pack_xbt(${xbt}/media ${CMAKE_BINARY_DIR}/${xbt}/media/Textures.xbt ${CMAKE_BINARY_DIR})
endforeach()
add_custom_target(pack-skins ALL
                  DEPENDS bootstrap export-files ${XBT_FILES})

set(CXIMAGE_DEPLIBS ${ZLIB_LIBRARIES} ${JPEG_LIBRARIES}
                    ${TIFF_LIBRARIES} ${PNG_LIBRARIES}
                    ${JASPER_LIBRARIES} )

xbmc_link_library(cximage system/ImageLib "nowrap" "${CXIMAGE_DEPLIBS}")
xbmc_link_library(exif system/libexif)
xbmc_link_library(hdhomerun system/hdhomerun)
xbmc_link_library(cmyth system/libcmyth "nowrap" ${MYSQLCLIENT_LIBRARIES})

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/system/players/paplayer)
xbmc_link_library(nosefart system/players/paplayer/nosefart)
xbmc_link_library(sidplay2 system/players/paplayer/libsidplay2)
xbmc_link_library(stsoundlibrary system/players/paplayer/stsoundlibrary)
xbmc_link_library(xbadpcm system/players/paplayer/adpcm)
xbmc_link_library(vgmstream system/players/paplayer/vgmstream)
xbmc_link_library(timidity
                  system/players/paplayer/timidity "export"
                  ${XBMC_SOURCE_DIR}/lib/timidity/exports.def)

xbmc_link_library(XBMC_addon addons/library.xbmc.addon/libXBMC_addon)
xbmc_link_library(XBMC_codec addons/library.xbmc.codec/libXBMC_codec)
xbmc_link_library(XBMC_gui   addons/library.xbmc.gui/libXBMC_gui)
xbmc_link_library(XBMC_pvr   addons/library.xbmc.pvr/libXBMC_pvr)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/system/players/dvdplayer)
add_custom_target(wrap-libraries ALL DEPENDS ${WRAP_FILES})

add_dependencies(xbmc.bin export-files)

set(prefix ${CMAKE_INSTALL_PREFIX})
configure_file(${XBMC_SOURCE_DIR}/xbmc/DllPaths_generated.h.in
               ${XBMC_BUILD_DIR}/DllPaths_generated.h @ONLY)

add_executable(xbmc-xrandr ${XBMC_SOURCE_DIR}/xbmc-xrandr.c)
target_link_libraries(xbmc-xrandr ${X_LIBRARIES} m ${XRANDR_LIBRARIES})

set(bindings ${XBMC_SOURCE_DIR}/xbmc/addons/include/xbmc_addon_cpp_dll.h
             ${XBMC_SOURCE_DIR}/xbmc/addons/include/xbmc_addon_cpp_dll.h
             ${XBMC_SOURCE_DIR}/xbmc/addons/include/xbmc_addon_dll.h
             ${XBMC_SOURCE_DIR}/xbmc/addons/include/xbmc_addon_types.h
             ${XBMC_SOURCE_DIR}/xbmc/addons/include/xbmc_codec_types.h
             ${XBMC_SOURCE_DIR}/xbmc/addons/include/xbmc_epg_types.h
             ${XBMC_SOURCE_DIR}/xbmc/addons/include/xbmc_pvr_dll.h
             ${XBMC_SOURCE_DIR}/xbmc/addons/include/xbmc_pvr_types.h
             ${XBMC_SOURCE_DIR}/xbmc/addons/include/xbmc_scr_dll.h
             ${XBMC_SOURCE_DIR}/xbmc/addons/include/xbmc_scr_types.h
             ${XBMC_SOURCE_DIR}/xbmc/addons/include/xbmc_vis_dll.h
             ${XBMC_SOURCE_DIR}/xbmc/addons/include/xbmc_vis_types.h
             ${XBMC_SOURCE_DIR}/xbmc/addons/include/xbmc_stream_utils.hpp
             ${XBMC_SOURCE_DIR}/addons/library.xbmc.addon/libXBMC_addon.h
             ${XBMC_SOURCE_DIR}/addons/library.xbmc.gui/libXBMC_gui.h
             ${XBMC_SOURCE_DIR}/addons/library.xbmc.pvr/libXBMC_pvr.h
             ${XBMC_SOURCE_DIR}/addons/library.xbmc.codec/libXBMC_codec.h
             ${XBMC_SOURCE_DIR}/xbmc/cores/dvdplayer/DVDDemuxers/DVDDemuxPacket.h)

file(GLOB cmake_files ${XBMC_SOURCE_DIR}/project/cmake/*.cmake)
configure_file(${XBMC_SOURCE_DIR}/project/cmake/xbmc-config.cmake.in
               ${XBMC_BUILD_DIR}/xbmc-config.cmake @ONLY)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  include(Scripts/XbmcInstallLinux.cmake)
endif()
