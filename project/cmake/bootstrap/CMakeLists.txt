project(xbmc-bootstrap)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)

find_package(Git REQUIRED)

function(add_git_copy id repo rev prefix)
  externalproject_add(${id}
                      GIT_REPOSITORY ${repo}
                      PREFIX build/${id}
                      GIT_TAG ${hash}
                      UPDATE_COMMAND ""
                      CONFIGURE_COMMAND ""
                      BUILD_COMMAND ""
                      INSTALL_COMMAND "")
  set(exclude "PATTERN .git EXCLUDE")
  if("${id}" MATCHES "^skin." AND BOOTSTRAP_IN_TREE)
    set(exclude "${exclude} PATTERN media EXCLUDE")
  endif()
  file(WRITE ${CMAKE_BINARY_DIR}/build/${id}-copy.cmake
       "FILE(COPY build/${id}/src/${id}
             DESTINATION ${OUTPUT_DIR}/${prefix} ${exclude})")
  add_custom_command(OUTPUT ${OUTPUT_DIR}/${prefix}/${id}
                     COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/build/${id}-copy.cmake
                     DEPENDS ${id})
endfunction()

function(add_git_cmake id repo rev prefix)
  externalproject_add(${id}
                      GIT_REPOSITORY ${repo}
                      PREFIX ${OUTPUT_DIR}/${prefix}/${id}
                      GIT_TAG ${hash}
                      UPDATE_COMMAND ""
                      CONFIGURE_COMMAND ""
                      BUILD_COMMAND ""
                      INSTALL_COMMAND "")
endfunction()

function(add_addon_separate entry)
  string(REPLACE " " ";" entry "${entry}")
  list(GET entry 0 id)
  list(GET entry 1 repo)
  list(GET entry 2 hash)
  if(BOOTSTRAP_DIR)
    if("${id}" MATCHES "^skin.")
      list(APPEND SKINS "${BOOTSTRAP_DIR}/addons/${id}\;${BOOTSTRAP_DIR}")
    elseif("${id}" MATCHES "^visualization" OR
           "${id}" MATCHES "^pvr.")
      list(APPEND build_dirs "${xbmc-platform_DEF}")
      list(REMOVE_DUPLICATES build_dirs)
    else()
      add_custom_command(OUTPUT ${OUTPUT_DIR}/addons/${id}
                         COMMAND cmake -E copy_directory ${BOOTSTRAP_DIR}}/addons/${id} ${OUTPUT_DIR}/addons/${id})
      list(APPEND SOURCES ${OUTPUT_DIR}/addons/${id})
      list(APPEND install_dirs ${OUTPUT_DIR}/addons/${id})
    endif()
  else()
    if("${id}" MATCHES "^visualization." OR
       "${id}" MATCHES "^pvr.")
      if(BOOTSTRAP_IN_TREE)
        list(APPEND build_dirs "${id} ${repo} ${hash}")
        list(APPEND build_dirs "${xbmc-platform_DEF}")
        list(REMOVE_DUPLICATES build_dirs)
      else()
        add_git_cmake(${id} ${repo} ${hash} addons)
        if(NOT TARGET xbmc-platform)
          add_git_cmake(xbmc-platform "${xbmc-platform_DEF}")
        endif()
      endif()
    else()
      add_git_copy(${id} ${repo} ${hash} addons)
      list(APPEND install_dirs ${OUTPUT_DIR}/addons/${id})
      set(install_dirs ${install_dirs} PARENT_SCOPE)
      list(APPEND SOURCES ${OUTPUT_DIR}/addons/${id})
    endif()
    if("${id}" MATCHES "^skin.")
      if(BOOTSTRAP_IN_TREE)
        list(APPEND EXTRA_XBT addons/${id})
      endif()
    endif()
  endif()
  set(EXTRA_XBT ${EXTRA_XBT} PARENT_SCOPE)
  set(build_dirs ${build_dirs} PARENT_SCOPE)
  set(install_dirs ${install_dirs} PARENT_SCOPE)
  set(SOURCES ${SOURCES} PARENT_SCOPE)
  set(SKINS ${SKINS} PARENT_SCOPE)
endfunction()

function(add_addon_multi lines)
  list(GET lines 0 base)
  list(GET lines 1 base2)
  list(REMOVE_ITEM lines ${base})
  string(REPLACE " " ";" base ${base})
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
                         COMMAND cmake -E copy_directory ${BOOTSTRAP_DIR}}/${entry} ${OUTPUT_DIR}/addons/${entry})
    else()
      add_custom_command(OUTPUT ${OUTPUT_DIR}/addons/${entry}
                         COMMAND cmake -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/build/${id}/src/${id}/${entry} ${OUTPUT_DIR}/addons/${entry}
                         DEPENDS ${id})
    endif()
    list(APPEND SOURCES ${OUTPUT_DIR}/addons/${entry})
    list(APPEND install_dirs ${OUTPUT_DIR}/addons/${entry})
    set(install_dirs ${install_dirs} PARENT_SCOPE)
  endforeach()
endfunction()

file(GLOB separate_input_files installdata/addons/common/*.txt)
file(GLOB sep2 installdata/addons/${CORE_SYSTEM_NAME}/*.txt)
list(APPEND separate_input_files ${sep2})
file(GLOB optional_input_files installdata/addons/optional/common/*.txt)
file(GLOB optsep2 installdata/addons/optional/${CORE_SYSTEM_NAME}/*.txt)
list(APPEND optional_input_files ${optsep2})

file(GLOB multi_input_files installdata/addons/multi/common/*.txt)
file(GLOB multi2 installdata/addons/multi/${CORE_SYSTEM_NAME}/*.txt)
list(APPEND multi_input_files ${multi2})

file(GLOB optional_multi_input_files installdata/addons/optional/multi/common/*.txt)
file(GLOB multiopt2 installdata/addons/optional/multi/${CORE_SYSTEM_NAME}/*.txt)
list(APPEND optional_multi_input_files ${multiopt2})

file(STRINGS installdata/addons/xbmc-platform.txt xbmc-platform_DEF)

if(NOT CORE_SOURCE_DIR)
  set(CORE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
endif()

if(NOT OUTPUT_DIR)
  set(OUTPUT_DIR ${CMAKE_BINARY_DIR})
endif()

# Add-ons hosted in a separate git
foreach(file ${separate_input_files})
  file(STRINGS ${file} entry)
  add_addon_separate(${entry})
endforeach()

# Optional add-ons hosted in a separate git
foreach(file ${optional_input_files})
  file(STRINGS ${file} entry)
  string(REPLACE " " ";" entry "${entry}")
  list(GET entry 0 opt)
  list(GET entry 1 def)
  list(GET entry 2 id)
  list(GET entry 3 repo)
  list(GET entry 4 hash)
  option(ENABLE_${opt} "Enable ${id}?" ${def})
  if(ENABLE_${opt})
    add_addon_separate("${id} ${repo} ${hash}"})
  endif()
endforeach()

# Several add-ons host in subdirs of a common git
foreach(file ${multi_input_files})
  file(STRINGS ${file} lines)
  string(REPLACE "\n" ";" lines "${lines}")
  add_addon_multi("${lines}")
endforeach()

# Optional multi-addons
foreach(file ${optional_multi_input_files})
  file(STRINGS ${file} lines2)
  string(REPLACE "\n" ";" lines "${lines2}")
  list(GET lines 0 base)
  string(REPLACE " " ";" base ${base})
  list(GET base 0 opt)
  list(GET base 1 def)
  list(GET base 2 id)
  list(GET base 3 repo)
  list(GET base 4 hash)
  option(ENABLE_${opt} "Enable ${id}?" ${def})
  if(ENABLE_${opt})
    string(REPLACE "\n" ";" lines "${lines2}")
    list(REMOVE_AT lines 0)
    list(INSERT lines 0 "${id} ${repo} ${hash}")
    add_addon_multi("${lines}")
  endif()
endforeach()

# Write a list of dirs to glob at installation time
string(REPLACE ";" "\n" install_dirs "${install_dirs}")
file(WRITE ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/extra-installs
           ${install_dirs})

add_subdirectory(${CORE_SOURCE_DIR}/xbmc/interfaces/swig swig.dir)

set(autoconf_dirs lib/libdvd/libdvdcss
                   lib/libdvd/libdvdread
                   lib/libdvd/libdvdnav
                   lib/cpluff)

foreach(dir ${autoconf_dirs})
  add_custom_command(OUTPUT ${CORE_SOURCE_DIR}/${dir}/config.h.in
                     COMMAND autoreconf -vif WORKING_DIRECTORY ${CORE_SOURCE_DIR}/${dir})
  list(APPEND SOURCES ${CORE_SOURCE_DIR}/${dir}/config.h.in)
endforeach()

add_custom_target(bootstrap ALL SOURCES ${SOURCES})

if(BOOTSTRAP_IN_TREE OR BOOTSTRAP_DIR)
  set(build_dirs ${build_dirs} PARENT_SCOPE)
  set(install_dirs ${install_dirs} PARENT_SCOPE)
  set(EXTRA_XBT ${EXTRA_XBT} PARENT_SCOPE)
  set(SKINS ${SKINS} PARENT_SCOPE)
endif()
