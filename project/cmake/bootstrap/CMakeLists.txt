project(xbmc-bootstrap)

cmake_minimum_required(VERSION 2.6)

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
 add_custom_command(OUTPUT ${OUTPUT_DIR}/${prefix}/${id}
                    COMMAND ${CMAKE_COMMAND} -E copy_directory build/${id}/src/${id} ${OUTPUT_DIR}/${prefix}/${id})
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

if(NOT OUTPUT_DIR)
  set(OUTPUT_DIR ${CMAKE_BINARY_DIR})
endif()

set(scrapers_git git://git.code.sf.net/p/xbmc/scrapers)
  externalproject_add(scrapers
                      GIT_REPOSITORY ${scrapers_git}
                      PREFIX build/scrapers
                      GIT_TAG ${scrapers_rev}
                      UPDATE_COMMAND ""
                      CONFIGURE_COMMAND ""
                      BUILD_COMMAND ""
                      INSTALL_COMMAND "")
foreach(scraper ${scrapers})
  add_custom_command(OUTPUT ${OUTPUT_DIR}/addons/${scraper}
                     COMMAND cmake -E copy_directory build/scrapers/${scraper} ${OUTPUT_DIR}/addons/${scraper}
                     DEPENDS scrapers)
  list(APPEND SOURCES ${OUTPUT_DIR}/addons/${scraper})
  list(APPEND install_data ${OUTPUT_DIR}/addons/${scraper})
endforeach()

foreach(git ${separate_addons})
  list(GET git 0 id)
  list(GET git 1 repo)
  list(GET git 2 hash)
  add_git(${id} ${repo} ${hash} addons)
  list(APPEND SOURCES ${OUTPUT_DIR}/addons/${id})
endforeach()

add_custom_target(bootstrap ALL SOURCES ${SOURCES})
