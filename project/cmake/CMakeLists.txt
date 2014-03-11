project(xbmc)

cmake_minimum_required(VERSION 2.8.8)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/modules)
if(DEPENDS_DIR)
  list(APPEND CMAKE_PREFIX_PATH ${DEPENDS_DIR})
endif()

enable_language(CXX)
enable_language(C)

# general
option(VERBOSE            "Enable verbose output?" OFF)
option(ENABLE_TESTS       "Enable tests?" ON)
option(ENABLE_DVDCSS      "Enable libdvdcss support?" ON)
option(ENABLE_UPNP        "Enable UPnP support?" ON)
option(ENABLE_MICROHTTPD  "Enable MicroHttpd webserver?" ON)
option(ENABLE_MYSQLCLIENT "Enable MySql support?" ON)
option(ENABLE_AVAHI       "Enable Avahi support?" ON)
option(ENABLE_RTMP        "Enable RTMP support?" ON)
option(ENABLE_BLURAY      "Enable BluRay support?" ON)
option(ENABLE_PLIST       "Enable AirPlay support?" ON)
option(ENABLE_AIRTUNES    "Enable AirTunes support?" ON)
option(ENABLE_GNUTLS      "Enable gnutls support?" ON)
option(ENABLE_EXTERNAL_FFMPEG "Enable external ffmpeg?" OFF)
option(ENABLE_EXTERNAL_LIBAV "Enable external libav?" OFF)
option(ENABLE_STATIC_FFMPEG "Link FFMPEG statically?" ON)
if(UNIX)
  option(ENABLE_ALSA        "Enable ALSA support?" ON)
  option(ENABLE_PULSEAUDIO  "Enable PulseAudio support?" ON)
  option(ENABLE_DBUS        "Enable dbus support?" ON)
  option(ENABLE_LIBUSB      "Enable libusb support?" ON)
  option(ENABLE_UDEV        "Enable UDev support?" ON)
  option(ENABLE_OPENGL      "Enable OpenGL?" ON)
  option(ENABLE_SDL         "Enable SDL?" ON)
  option(ENABLE_X11         "Enable X11 support?" ON)
  option(ENABLE_WAYLAND     "Enable Wayland support?" OFF)
  option(ENABLE_EGL         "Enable EGL?" OFF)
  option(ENABLE_VAAPI       "Enable VAAPI support?" ON)
  option(ENABLE_VDPAU       "Enable VDPAU support?" ON)
  option(ENABLE_OPTICAL     "Enable optical support?" ON)
endif()
# System options
option(WITH_ARCH              "build with given arch" OFF)
option(WITH_CPU               "build with given cpu" OFF)

set(CORE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../..)
set(CORE_BUILD_DIR build)
if(BOOTSTRAP_DIR)
  set(UPDATE_COMMAND "")
else()
  set(UPDATE_COMMAND autoreconf -vif)
endif()

include(scripts/common/addoptions.cmake)
include(scripts/common/archsetup.cmake)
include(scripts/common/macros.cmake)
include(scripts/common/managestring.cmake)
include(scripts/common/projectmacros.cmake)
include(scripts/common/pathsetup.cmake)
include(ExternalProject)

set(INCLUDES ${CORE_SOURCE_DIR}/addons/library.xbmc.addon
             ${CORE_SOURCE_DIR}/addons/library.xbmc.codec
             ${CORE_SOURCE_DIR}/addons/library.xbmc.gui
             ${CORE_SOURCE_DIR}/addons/library.xbmc.pvr
             ${CORE_SOURCE_DIR}/lib
             ${CORE_SOURCE_DIR}/lib/gtest/include
             ${CORE_SOURCE_DIR}/xbmc
             ${CORE_SOURCE_DIR}/xbmc/linux
             ${CORE_SOURCE_DIR}/xbmc/cores/dvdplayer
             ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR})

find_package(PkgConfig)
if(CMAKE_CROSSCOMPILING)
  if(PKG_CONFIG_EXECUTABLE)
    message(STATUS "CMAKE_CROSSCOMPILING: ${CMAKE_CROSSCOMPILING}")
    set(PKG_CONFIG_FOUND TRUE)
  endif()
endif()
find_package(Threads REQUIRED)
list(APPEND DEPLIBS ${CMAKE_THREAD_LIBS_INIT})

# Required dependencies
set(required_deps Sqlite3 FreeType PCRE Cpluff OpenSSL
                  TinyXML Python Yajl LibSmbClient ZLIB Xslt
                  JPEG Lzo2 Fribidi SampleRate TagLib FFMPEG)

# Optional dependencies
set(optional_deps MicroHttpd MySqlClient
                  Alsa UDev Dbus Avahi Wayland
                  PulseAudio VDPAU VAAPI)

# Required, dyloaded deps
set(required_dyload Curl MAD ASS Mpeg2 LibDvd)

# Optional, dyloaded deps
set(dyload_optional RTMP CEC Bluray Plist)

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

foreach(dep ${dyload_optional})
  core_optional_dyload_dep(${dep})
endforeach()

if(ENABLE_UPNP)
  list(APPEND DEP_DEFINES "-DUSE_UPNP=1")
endif()

if(ENABLE_OPTICAL)
  list(APPEND DEP_DEFINES -DHAS_DVD_DRIVE)
  core_require_dep(Cdio)
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

core_find_git_rev()

include_directories(${INCLUDES} ${SYSTEM_INCLUDES})
add_definitions(${ARCH_DEFINES} ${SYSTEM_DEFINES} ${DEP_DEFINES} ${PATH_DEFINES})

set(core_DEPENDS "" CACHE STRING "" FORCE)
set(test_archives "" CACHE STRING "" FORCE)
set(test_sources "" CACHE STRING "" FORCE)
mark_as_advanced(core_DEPENDS)
mark_as_advanced(test_archives)
mark_as_advanced(test_sources)

file(STRINGS ${PROJECT_SOURCE_DIR}/installdata/addon-bindings.txt bindings)
foreach(binding ${bindings})
  list(APPEND addon_bindings ${CORE_SOURCE_DIR}/${binding})
endforeach()

if(NOT BOOTSTRAP_DIR)
  set(BOOTSTRAP_IN_TREE 1)
endif()
add_subdirectory(bootstrap ${CORE_BUILD_DIR}/bootstrap)
add_subdirectory(${CORE_SOURCE_DIR}/lib/gtest ${CORE_BUILD_DIR}/gtest)

# Subdirs
core_add_subdirs_from_filelist(${PROJECT_SOURCE_DIR}/treedata/common/*.txt
                               ${PROJECT_SOURCE_DIR}/treedata/${CORE_SYSTEM_NAME}/*.txt)
core_add_optional_subdirs_from_filelist(${PROJECT_SOURCE_DIR}/treedata/optional/common/*.txt
                                        ${PROJECT_SOURCE_DIR}/treedata/optional/${CORE_SYSTEM_NAME}/*.txt)

# main binary
add_executable(xbmc.bin ${CORE_SOURCE_DIR}/xbmc/main/main.cpp)
target_link_libraries(xbmc.bin ${SYSTEM_LDFLAGS} -Wl,--whole-archive ${core_DEPENDS} -Wl,--no-whole-archive
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
core_link_library(cmyth system/libcmyth "nowrap" ${MYSQLCLIENT_LIBRARIES})

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

# For generating the doxy
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CORE_SOURCE_DIR}/doxygen_resources/Doxyfile.doxy ${CORE_BUILD_DIR}/Doxyfile @ONLY)
  add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/${CORE_BUILD_DIR}/Doxyfile
                    WORKING_DIRECTORY ${CORE_SOURCE_DIR}/doxygen_resources
                    COMMENT "Generating API documentation" VERBATIM)
endif()

if(ENABLE_TESTS)
  copy_files_from_filelist_to_buildtree(${PROJECT_SOURCE_DIR}/installdata/test-reference-data.txt 1)
  add_executable(xbmc-test ${CORE_SOURCE_DIR}/xbmc/test/xbmc-test.cpp)
  target_link_libraries(xbmc-test ${SYSTEM_LDFLAGS} -Wl,--whole-archive ${core_DEPENDS} gtest ${test_archives} -Wl,--no-whole-archive
                        ${DEPLIBS} ${CMAKE_DL_LIBS})
  enable_testing()
  gtest_add_tests(xbmc-test "" ${test_sources})
endif()

set(cmake-files ${CORE_SOURCE_DIR}/project/cmake/scripts/common/addoptions.cmake
                ${CORE_SOURCE_DIR}/project/cmake/scripts/common/xbmc-addon-helpers.cmake)
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
  set(shared 1)
  set(install_prefix ${CMAKE_INSTALL_PREFIX})
  set(install_command INSTALL_COMMAND "")
  if(subdir MATCHES xbmc-platform)
    set(shared 0)
    set(install_prefix ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR})
    set(install_command)
  endif()
  if(BOOTSTRAP_IN_TREE)
    externalproject_add(${id}
                        GIT_REPOSITORY ${url}
                        GIT_TAG ${hash}
                        PREFIX ${CORE_BUILD_DIR}/${id}
                        CONFIGURE_COMMAND PKG_CONFIG_PATH=${DEPENDS_DIR}/lib/pkgconfig
                                          ${CMAKE_COMMAND} <SOURCE_DIR>
                                          -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
                                          -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                          -DBUILD_SHARED_LIBS=${shared}
                                          -DCMAKE_INSTALL_PREFIX=${install_prefix}
                                          -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                                          -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                                          -DXBMC_BUILD_DIR=${CMAKE_BINARY_DIR}
                                          -DBOOTSTRAP_IN_TREE=1
                        "${install_command}")
  else()
    externalproject_add(${id}
                        SOURCE_DIR ${BOOTSTRAP_DIR}/addons/compile/${id}/src/${id}
                        PREFIX ${CORE_BUILD_DIR}/${id}
                        CONFIGURE_COMMAND PKG_CONFIG_PATH=${DEPENDS_DIR}/lib/pkgconfig
                                          ${CMAKE_COMMAND} <SOURCE_DIR>
                                          -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
                                          -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                          -DBUILD_SHARED_LIBS=${shared}
                                          -DCMAKE_INSTALL_PREFIX=${install_prefix}
                                          -DXBMC_BUILD_DIR=${CMAKE_BINARY_DIR}
                                          -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                                          -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                        "${install_command}")
  endif()
  add_dependencies(${id} xbmc-platform)
endforeach()

list(APPEND cmake-files ${CORE_BUILD_DIR}/xbmc-config.cmake)
include(scripts/${CORE_SYSTEM_NAME}/install.cmake)

# Status
message("#---- CONFIGURATION ----#")
foreach(msg ${final_message})
  message(STATUS ${msg})
endforeach()
message("#-----------------------#")
