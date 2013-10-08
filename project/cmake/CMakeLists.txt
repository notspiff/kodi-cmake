project(xbmc)

cmake_minimum_required(VERSION 2.8.8)

include(ExternalProject)
include(scripts/AddOptions.cmake)
include(scripts/Macros.cmake)
include(scripts/ManageString.cmake)
include(scripts/ProjectMacros.cmake)
include(scripts/PathSetup.cmake)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/modules)

enable_language(CXX)
enable_language(C)

# general
option(VERBOSE            "Enable verbose output?" OFF)
option(ENABLE_DVDCSS      "Enable libdvdcss support?" ON)
option(ENABLE_UPNP        "Enable UPnP support?" ON)
option(ENABLE_NONFREE     "Enable non-free components?" ON)
option(ENABLE_MICROHTTPD  "Enable MicroHttpd webserver?" ON)
option(ENABLE_LAME        "Enable LAME mp3 encoder?" ON)
option(ENABLE_MYSQLCLIENT "Enable MySql support?" ON)
option(ENABLE_AVAHI       "Enable Avahi support?" ON)
option(ENABLE_RTMP        "Enable RTMP support?" ON)
option(ENABLE_BLURAY      "Enable BluRay support?" ON)
option(ENABLE_AFPCLIENT   "Enable AFP support?" ON)
option(ENABLE_PLIST       "Enable AirPlay support?" ON)
option(ENABLE_AIRTUNES    "Enable AirTunes support?" ON)
option(ENABLE_SSH         "Enable SSH support?" ON)
option(ENABLE_NFS         "Enable NFS support?" ON)
option(ENABLE_GNUTLS      "Enable gnutls support?" ON)
option(ENABLE_EXTERNAL_FFMPEG "Enable external ffmpeg?" OFF)
option(ENABLE_EXTERNAL_LIBAV "Enable external libav?" OFF)
if(UNIX)
  option(ENABLE_ALSA        "Enable ALSA support?" ON)
  option(ENABLE_PULSEAUDIO  "Enable PulseAudio support?" OFF)
  option(ENABLE_DBUS        "Enable dbus support?" ON)
  option(ENABLE_LIBUSB      "Enable libusb support?" ON)
  option(ENABLE_UDEV        "Enable UDev support?" ON)
  option(ENABLE_OPENGL      "Enable OpenGL?" ON)
  option(ENABLE_SDL         "Enable SDL?" ON)
  option(ENABLE_X11         "Enable X11 support?" ON)
  option(ENABLE_VAAPI       "Enable VAAPI support?" ON)
  option(ENABLE_VDPAU       "Enable VDPAU support?" ON)
endif()
# System options
option(ENABLE_CROSSCOMPILE    "Enable Cross_compiling?" OFF)
option(WITH_ARCH              "build with given arch" OFF)
option(WITH_CPU               "build with given cpu" OFF)

set(CORE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../..)
set(CORE_BUILD_DIR build)
if(BOOTSTRAP_DIR)
  set(UPDATE_COMMAND "")
else()
  set(UPDATE_COMMAND autoreconf -vif)
endif()

include(scripts/ArchSetup.cmake)

set(INCLUDES ${CORE_SOURCE_DIR}/addons/library.xbmc.addon
             ${CORE_SOURCE_DIR}/addons/library.xbmc.codec
             ${CORE_SOURCE_DIR}/addons/library.xbmc.gui
             ${CORE_SOURCE_DIR}/addons/library.xbmc.pvr
             ${CORE_SOURCE_DIR}/lib
             ${CORE_SOURCE_DIR}/xbmc
             ${CORE_SOURCE_DIR}/xbmc/linux
             ${CORE_SOURCE_DIR}/xbmc/cores/dvdplayer
             ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR})

find_package(PkgConfig)
find_package(Threads REQUIRED)
list(APPEND DEPLIBS ${CMAKE_THREAD_LIBS_INIT})

# Required dependencies
set(required_deps Sqlite3 FreeType PCRE Cpluff OpenSSL
                  TinyXML Python Yajl LibSmbClient ZLIB
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
set(required_dep_libs Jasper PNG TIFF EXPAT)

# Optional libs for shared objects we link
set(optional_dep_libs GnuTls)

foreach(dep ${required_dep_libs})
  find_package(${dep} REQUIRED)
endforeach()

foreach(dep ${optional_dep_libs})
  string(TOUPPER dep depup)
  if(ENABLE_${depup})
    find_package(${dep})
  endif()
endforeach()

foreach(dep ${required_deps})
  core_require_dep(${dep})
endforeach()

foreach(dep ${optional_deps})
  core_optional_dep(${dep})
endforeach()

if(NOT UDEV_FOUND)
  core_optional_dep(LibUSB)
endif()

foreach(dep ${required_dyload})
  core_require_dyload_dep(${dep})
endforeach()

# Need a special case due to upper case lib name
find_package(FLAC REQUIRED)
find_soname(FLAC FLAC)

foreach(dep ${dyload_optional})
  core_optional_dyload_dep(${dep})
endforeach()

if(ENABLE_UPNP)
  list(APPEND DEP_DEFINES "-DUSE_UPNP=1")
endif()

if(ENABLE_NONFREE)
  list(APPEND DEP_DEFINES "-DHAVE_XBMC_NONFREE=1")
endif()

core_optional_dep(Shairplay ENABLE_AIRTUNES)
if(NOT SHAIRPLAY_FOUND)
  core_optional_dep(Shairport ENABLE_AIRTUNES)
endif()

if(NOT WIN32)
  core_optional_dep(OpenGl)
  if(OPENGL_FOUND)
    core_require_dep(GLEW)
    core_optional_dep(X ENABLE_X11)
    core_optional_dep(XRandR ENABLE_X11)
    core_optional_dep(Sdl)
  endif()
endif()

if(ENABLE_SKIN_TOUCHED)
  list(APPEND DEP_DEFINES "-DHAS_SKIN_TOUCHED=1")
endif()

include_directories(${INCLUDES} ${SYSTEM_INCLUDES})
add_definitions(${ARCH_DEFINES} ${SYSTEM_DEFINES} ${DEP_DEFINES} ${PATH_DEFINES})

set(xbmc-bin_DEPENDS "" CACHE STRING "" FORCE)
mark_as_advanced(xbmc-bin_DEPENDS)

file(STRINGS ${PROJECT_SOURCE_DIR}/installdata/addon-bindings.txt bindings)
foreach(binding ${bindings})
  list(APPEND addon_bindings ${CORE_SOURCE_DIR}/${binding})
endforeach()

if(NOT BOOTSTRAP_DIR)
  set(BOOTSTRAP_IN_TREE 1)
endif()
add_subdirectory(bootstrap ${CORE_BUILD_DIR}/bootstrap.dir)

# Subdirs
core_add_subdirs_from_filelist(${PROJECT_SOURCE_DIR}/treedata/common/*.txt
                               ${PROJECT_SOURCE_DIR}/treedata/${CORE_SYSTEM_NAME}/*.txt)
core_add_optional_subdirs_from_filelist(${PROJECT_SOURCE_DIR}/treedata/optional/common/*.txt;
                                        ${PROJECT_SOURCE_DIR}/treedata/optional/${CORE_SYSTEM_NAME}/*.txt)

# main binary
add_executable(xbmc.bin ${CORE_SOURCE_DIR}/xbmc/main/main.cpp)
target_link_libraries(xbmc.bin ${SYSTEM_LDFLAGS} -Wl,--whole-archive ${xbmc-bin_DEPENDS} -Wl,--no-whole-archive
                      ${DEPLIBS} ${CMAKE_DL_LIBS})
install(TARGETS xbmc.bin DESTINATION ${libdir}/xbmc)

# copy files to build tree
copy_files_from_filelist_to_buildtree(${PROJECT_SOURCE_DIR}/installdata/common/*.txt
                                      ${PROJECT_SOURCE_DIR}/installdata/${CORE_SYSTEM_NAME}/*.txt)

list(APPEND SKINS "${CORE_SOURCE_DIR}/addons/skin.confluence\;${CORE_SOURCE_DIR}")

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

core_link_library(cximage system/ImageLib "nowrap" "${CXIMAGE_DEPLIBS}")
core_link_library(exif system/libexif)
core_link_library(hdhomerun system/hdhomerun)
core_link_library(cmyth system/libcmyth "nowrap" ${MYSQLCLIENT_LIBRARIES})

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/system/players/paplayer)
core_link_library(nosefart system/players/paplayer/nosefart)
core_link_library(sidplay2 system/players/paplayer/libsidplay2)
core_link_library(stsoundlibrary system/players/paplayer/stsoundlibrary)
core_link_library(xbadpcm system/players/paplayer/adpcm)
core_link_library(vgmstream system/players/paplayer/vgmstream)
core_link_library(timidity
                  system/players/paplayer/timidity "export"
                  ${CORE_SOURCE_DIR}/lib/timidity/exports.def)

core_link_library(XBMC_addon addons/library.xbmc.addon/libXBMC_addon)
core_link_library(XBMC_codec addons/library.xbmc.codec/libXBMC_codec)
core_link_library(XBMC_gui   addons/library.xbmc.gui/libXBMC_gui)
core_link_library(XBMC_pvr   addons/library.xbmc.pvr/libXBMC_pvr)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/system/players/dvdplayer)
add_custom_target(wrap-libraries ALL DEPENDS ${WRAP_FILES})

add_dependencies(xbmc.bin export-files)

configure_file(${CORE_SOURCE_DIR}/xbmc/DllPaths_generated.h.in
               ${CORE_BUILD_DIR}/DllPaths_generated.h @ONLY)

add_executable(xbmc-xrandr ${CORE_SOURCE_DIR}/xbmc-xrandr.c)
target_link_libraries(xbmc-xrandr ${SYSTEM_LDFLAGS} ${X_LIBRARIES} m ${XRANDR_LIBRARIES})

set(cmake-files ${CORE_SOURCE_DIR}/project/cmake/scripts/AddOptions.cmake
                ${CORE_SOURCE_DIR}/project/cmake/scripts/xbmc-addon-helpers.cmake)
configure_file(${CORE_SOURCE_DIR}/project/cmake/templates/xbmc-config.cmake.in
               ${CORE_BUILD_DIR}/modules/xbmc-config.cmake @ONLY)

# These for built extras
if(build_dirs)
  prepare_addon_env()
endif()
foreach(subdir ${build_dirs})
  string(REPLACE " " ";" subdir ${subdir})
  list(GET subdir 0 id)
  list(GET subdir 1 url)
  list(GET subdir 2 hash)
  if(BOOTSTRAP_IN_TREE)
    externalproject_add(${id}
                        GIT_REPOSITORY ${url}
                        GIT_TAG ${hash}
                        PREFIX ${CORE_BUILD_DIR}/${id}
                        CMAKE_ARGS -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
                        -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
                        -DBUILD_SHARED_LIBS=1
                        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                        INSTALL_COMMAND "")
    set(dest_path ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/${id}/src)
  else()
    externalproject_add(${id}
                        SOURCE_DIR ${BOOTSTRAP_DIR}/addons/${id}/src/${id}
                        PREFIX ${CORE_BUILD_DIR}/${id}
                        CMAKE_ARGS -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
                        -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
                        -DBUILD_SHARED_LIBS=1
                        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                        INSTALL_COMMAND "")
   set(dest_path ${BOOTSTRAP_DIR}/addons/${id}/src/${id})
  endif()
  add_dependencies(${id} xbmc-platform)
endforeach()

list(APPEND cmake-files ${CORE_BUILD_DIR}/xbmc-config.cmake)
include(scripts/Install${CMAKE_SYSTEM_NAME}.cmake)

# Status
message("#---- CONFIGURATION ----#")
foreach(msg ${final_message})
  message(STATUS ${msg})
endforeach()
message("#-----------------------#")

