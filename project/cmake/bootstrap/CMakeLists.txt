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

list(APPEND separate_input_files separates.txt)
list(APPEND multi_dirs scrapers.txt)

if(ENABLE_SKIN_TOUCHED)
  list(APPEND separate_input_files skin.touched.txt)
endif()

if(NOT XBMC_SOURCE_DIR)
  set(XBMC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
endif()

if(NOT OUTPUT_DIR)
  set(OUTPUT_DIR ${CMAKE_BINARY_DIR})
endif()

foreach(file ${multi_dirs})
  file(STRINGS ${file} lines)
  string(REPLACE "\n" ";" lines "${lines}")
  list(GET lines 0 base)
  string(REPLACE " " ";" base ${base})
  list(REMOVE_AT lines 0)
  list(GET base 0 id)
  list(GET base 1 repo)
  list(GET base 2 hash)
  if(NOT BOOTSTRAP_DIR)
    externalproject_add(${id}
                        GIT_REPOSITORY ${repo}
                        PREFIX build/${id}
                        GIT_TAG ${hash}
                        UPDATE_COMMAND ""
                        CONFIGURE_COMMAND ""
                        BUILD_COMMAND ""
                        INSTALL_COMMAND "")
    endif()
  foreach(entry ${lines})
    if(BOOTSTRAP_DIR)
      add_custom_command(OUTPUT ${OUTPUT_DIR}/addons/${entry}
                         COMMAND cmake -E copy_directory ${BOOTSTRAP_DIR}}/${entry} ${OUTPUT_DIR}/addons/${entry}
                         COMMAND cmake -E touch ${OUTPUT_DIR}/addons/${entry})
    else()
      add_custom_command(OUTPUT ${OUTPUT_DIR}/addons/${entry}
                         COMMAND cmake -E copy_directory build/${id}/${entry} ${OUTPUT_DIR}/addons/${entry}
                         COMMAND cmake -E touch ${OUTPUT_DIR}/addons/${entry}
                         DEPENDS ${id})
    endif()
    list(APPEND SOURCES ${OUTPUT_DIR}/addons/${entry})
    list(APPEND install_data ${OUTPUT_DIR}/addons/${entry})
  endforeach()
endforeach()

foreach(file ${separate_input_files})
  file(STRINGS ${file} entries)
  string(REPLACE "\n" ";" entries "${entries}")
  foreach(entry ${entries})
    string(REPLACE " " ";" entry ${entry})
    list(GET entry 0 id)
    list(GET entry 1 repo)
    list(GET entry 2 hash)
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
