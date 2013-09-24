project(xbmc-bootstrap)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)

find_package(Git REQUIRED)

function(add_git id repo rev prefix)
  externalproject_add(${id}
                      GIT_REPOSITORY ${repo}
                      PREFIX build/${id}
                      GIT_TAG ${hash}
                      UPDATE_COMMAND ""
                      CONFIGURE_COMMAND ""
                      BUILD_COMMAND ""
                      INSTALL_COMMAND "")
  set(exclude "PATTERN .git EXCLUDE")
  if("${id}" MATCHES "^skin" AND BOOTSTRAP_IN_TREE)
    list(APPEND exclude "PATTERN media EXCLUDE")
  endif()
  file(WRITE ${CMAKE_BINARY_DIR}/build/${id}-copy.cmake "FILE(COPY build/${id}/src/${id}
                                     DESTINATION ${OUTPUT_DIR}/${prefix}
                                     ${exclude})")
  add_custom_command(OUTPUT ${OUTPUT_DIR}/${prefix}/${id}
                     COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/build/${id}-copy.cmake
                     DEPENDS ${id})
endfunction()

set(separate_addons
   "service.xbmc.versioncheck\;git://github.com/XBMC-Addons/service.xbmc.versioncheck\;a78723bea"
)

if(ENABLE_SKIN_TOUCHED)
  list(APPEND separate_addons "skin.touched\;git://github.com/xbmc/skin.touched.git\;2ff9d29cd")
endif()

set(scrapers metadata.album.universal
             metadata.album.universal
             metadata.artists.universal
             metadata.common.allmusic.com
             metadata.common.amazon.de
             metadata.common.fanart.tv
             metadata.common.hdtrailers.net
             metadata.common.htbackdrops.com
             metadata.common.imdb.com
             metadata.common.last.fm
             metadata.common.musicbrainz.org
             metadata.common.theaudiodb.com
             metadata.common.themoviedb.org
             metadata.musicvideos.theaudiodb.com
             metadata.themoviedb.org
             metadata.tvdb.com)

set(scrapers_rev 8e0d639)

if(NOT XBMC_SOURCE_DIR)
  set(XBMC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
endif()

if(NOT OUTPUT_DIR)
  set(OUTPUT_DIR ${CMAKE_BINARY_DIR})
endif()

set(scrapers_git git://git.code.sf.net/p/xbmc/scrapers)
if(NOT BOOTSTRAP_DIR)
  externalproject_add(scrapers
                      GIT_REPOSITORY ${scrapers_git}
                      PREFIX build/scrapers
                      GIT_TAG ${scrapers_rev}
                      UPDATE_COMMAND ""
                      CONFIGURE_COMMAND ""
                      BUILD_COMMAND ""
                      INSTALL_COMMAND "")
endif()

foreach(scraper ${scrapers})
  if(BOOTSTRAP_DIR)
    add_custom_command(OUTPUT ${OUTPUT_DIR}/addons/${scraper}
                       COMMAND cmake -E copy_directory ${BOOTSTRAP_DIR}}/${scraper} ${OUTPUT_DIR}/addons/${scraper}
                       COMMAND cmake -E touch ${OUTPUT_DIR}/addons/${scraper})
  else()
    add_custom_command(OUTPUT ${OUTPUT_DIR}/addons/${scraper}
                       COMMAND cmake -E copy_directory build/scrapers/${scraper} ${OUTPUT_DIR}/addons/${scraper}
                       COMMAND cmake -E touch ${OUTPUT_DIR}/addons/${scraper}
                       DEPENDS scrapers)
  endif()
  list(APPEND SOURCES ${OUTPUT_DIR}/addons/${scraper})
  list(APPEND install_data ${OUTPUT_DIR}/addons/${scraper})
endforeach()

foreach(git ${separate_addons})
  list(GET git 0 id)
  list(GET git 1 repo)
  list(GET git 2 hash)
  if(BOOTSTRAP_DIR)
    if("${id}" MATCHES "^skin")
      list(APPEND SKINS "${BOOTSTRAP_DIR}/addons/${id}\;${BOOTSTRAP_DIR}")
      set(SKINS "${SKINS}" PARENT_SCOPE)
    else()
      add_custom_command(OUTPUT ${OUTPUT_DIR}/addons/${id}
                         COMMAND cmake -E copy_directory ${BOOTSTRAP_DIR}}/addons/${id} ${OUTPUT_DIR}/addons/${id})
      list(APPEND SOURCES ${OUTPUT_DIR}/addons/${id})
    endif()
  else()
    add_git(${id} ${repo} ${hash} addons)
    list(APPEND SOURCES ${OUTPUT_DIR}/addons/${id})
    if("${id}" MATCHES "^skin")
      if(BOOTSTRAP_IN_TREE)
        list(APPEND EXTRA_XBT addons/${id})
        set(EXTRA_XBT ${EXTRA_XBT} PARENT_SCOPE)
      endif()
    endif()
  endif()
endforeach()

add_subdirectory(${XBMC_SOURCE_DIR}/xbmc/interfaces/swig swig.dir)

set(autoconf_dirs lib/libdvd/libdvdcss
                   lib/libdvd/libdvdread
                   lib/libdvd/libdvdnav
                   lib/cpluff)

foreach(dir ${autoconf_dirs})
  add_custom_command(OUTPUT ${XBMC_SOURCE_DIR}/${dir}/config.h.in
                     COMMAND autoreconf -vif WORKING_DIRECTORY ${XBMC_SOURCE_DIR}/${dir})
  list(APPEND SOURCES ${XBMC_SOURCE_DIR}/${dir}/config.h.in)
endforeach()

add_custom_target(bootstrap ALL SOURCES ${SOURCES})
