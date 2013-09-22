project(xbmc-bootstrap)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
include(../scripts/common/managestring.cmake)
if(NOT CORE_SYSTEM_NAME)
  string(TOLOWER ${CMAKE_SYSTEM_NAME} CORE_SYSTEM_NAME)
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/modules
                              ${PROJECT_SOURCE_DIR}/../scripts/common)

if(NOT CORE_BUILD_DIR)
  set(CORE_BUILD_DIR build)
endif()

find_package(Git REQUIRED)
find_package(AutoConf REQUIRED)

macro(add_git_copy id repo rev prefix)
  externalproject_add(${id}
                      GIT_REPOSITORY ${repo}
                      PREFIX build/${id}
                      GIT_TAG ${rev}
                      UPDATE_COMMAND ""
                      CONFIGURE_COMMAND ""
                      BUILD_COMMAND ""
                      INSTALL_COMMAND "")
  set(exclude "PATTERN .git EXCLUDE")
  if("${id}" MATCHES "^skin." AND BOOTSTRAP_IN_TREE)
    set(exclude "${exclude} PATTERN media EXCLUDE")
  endif()
  if(BUILDTOOLS)
    add_dependencies(${id} ${BUILDTOOLS})
  endif()
  file(WRITE ${CMAKE_BINARY_DIR}/build/${id}-copy.cmake
       "file(COPY ${CMAKE_CURRENT_BINARY_DIR}/build/${id}/src/${id}
             DESTINATION ${OUTPUT_DIR}/${prefix} ${exclude})")
  add_custom_command(OUTPUT ${OUTPUT_DIR}/${prefix}/${id}
                     COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/build/${id}-copy.cmake
                     DEPENDS ${id})
endmacro()

macro(setup_patching)
  file(STRINGS ${file} def)
  get_filename_component(dir ${file} PATH)
  string(REPLACE " " ";" def ${def})
  list(GET def 0 id)
  list(GET def 1 url)
  set(extraflags "")
  if(EXISTS ${dir}/flags.txt)
    file(STRINGS ${dir}/flags.txt extraflags)
    string(REPLACE " " ";" extraflags ${extraflags})
    set(extraflags "${extraflags}")
  endif()
  set(PATCH_COMMAND "")
  file(REMOVE ${OUTPUT_DIR}/${CORE_BUILD_DIR}/${id}/tmp/patch.cmake)
  if(EXISTS ${dir}/CMakeLists.txt)
    file(APPEND ${OUTPUT_DIR}/${CORE_BUILD_DIR}/${id}/tmp/patch.cmake
                "file(COPY ${dir}/CMakeLists.txt DESTINATION ${OUTPUT_DIR}/${CORE_BUILD_DIR}/${id}/src/${id})\n")
    set(PATCH_COMMAND ${CMAKE_COMMAND} -P ${OUTPUT_DIR}/${CORE_BUILD_DIR}/${id}/tmp/patch.cmake)
  endif()
  file(GLOB patches ${dir}/*.patch)
  list(SORT patches)
  foreach(patch ${patches})
    set(PATCH_COMMAND ${CMAKE_COMMAND} -P ${OUTPUT_DIR}/${CORE_BUILD_DIR}/${id}/tmp/patch.cmake)
    file(APPEND ${OUTPUT_DIR}/${CORE_BUILD_DIR}/${id}/tmp/patch.cmake
         "execute_process(COMMAND patch -p1 -i ${patch})\n")
  endforeach()
  if(EXISTS ${dir}/install.txt)
    set(INSTALL_COMMAND INSTALL_COMMAND ${CMAKE_COMMAND}
                                        -DINPUTDIR=${OUTPUT_DIR}/${CORE_BUILD_DIR}/${id}/src/${id}-build/
                                        -DINPUTFILE=${dir}/install.txt
                                        -DDESTDIR=${OUTPUT_DIR}
                                        "${extraflags}"
                                        -P ${PROJECT_SOURCE_DIR}/install.cmake)
  else()
    set(INSTALL_COMMAND "")
  endif()
endmacro()

function(add_git_lib file)
  file(STRINGS ${file} lib_DEF)
  string_split(lib_list " " "${lib_DEF}")
  list(GET lib_list 0 id)
  list(GET lib_list 1 repo)
  list(GET lib_list 2 hash)
  list(LENGTH lib_list len)
  if(len GREATER 3)
    list(GET lib_list 3 deps)
  endif()
  setup_patching()
  if(BOOTSTRAP_IN_TREE)
    externalproject_add(${id}
                        GIT_REPOSITORY ${repo}
                        GIT_TAG ${hash}
                        UPDATE_COMMAND ${CMAKE_COMMAND}
                                      -DROOT_DIR=${OUTPUT_DIR}/${CORE_BUILD_DIR}/${id}/src/${id}
                                      -P ${PROJECT_SOURCE_DIR}/installdata/bootstrap.cmake
                        PATCH_COMMAND ${PATCH_COMMAND}
                        CMAKE_ARGS -DCMAKE_PREFIX_PATH=${OUTPUT_DIR}/${CORE_BUILD_DIR}
                                 -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                 -DBUILD_SHARED_LIBS=0
                                 -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}/${CORE_BUILD_DIR}
                                 -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                                 -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                                 -DCMAKE_CXX_FLAGS=-fPIC
                                 -DCMAKE_C_FLAGS=-fPIC
                                 -DKODI_BUILD_DIR=${OUTPUT_DIR}
                        DEPENDS compileinfo
                        PREFIX ${OUTPUT_DIR}/${CORE_BUILD_DIR}/${id})
  elseif(BOOTSTRAP_DIR)
    externalproject_add(${id}
                        SOURCE_DIR ${BOOTSTRAP_DIR}/${CORE_BUILD_DIR}/${id}/src/${id}
                        PREFIX ${CORE_BUILD_DIR}/${id}
                        CONFIGURE_COMMAND PKG_CONFIG_PATH=${DEPENDS_DIR}/lib/pkgconfig
                                          ${CMAKE_COMMAND} <SOURCE_DIR>
                                          -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
                                          -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                          -DBUILD_SHARED_LIBS=0
                                          -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}/${CORE_BUILD_DIR}
                                          -DKODI_BUILD_DIR=${CMAKE_BINARY_DIR}
                                          -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                                          -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER})
  else()
    externalproject_add(${id}
                        GIT_REPOSITORY ${repo}
                        GIT_TAG ${hash}
                        UPDATE_COMMAND ${CMAKE_COMMAND}
                                      -DROOT_DIR=${OUTPUT_DIR}/${CORE_BUILD_DIR}/${id}/src/${id}
                                      -P ${PROJECT_SOURCE_DIR}/installdata/bootstrap.cmake
                        PATCH_COMMAND ${PATCH_COMMAND}
                        PREFIX ${CMAKE_BINARY_DIR}/build/${id}
                        CONFIGURE_COMMAND ""
                        BUILD_COMMAND ""
                        INSTALL_COMMAND "")
  endif()
  if(deps)
    add_dependencies(${id} ${deps})
  endif()
  if(BUILDTOOLS)
    add_dependencies(${id} ${BUILDTOOLS})
  endif()
endfunction()

function(add_git_cmake id repo hash)
  if(BOOTSTRAP_IN_TREE)
    externalproject_add(${id}
                        GIT_REPOSITORY ${repo}
                        GIT_TAG ${hash}
                        PREFIX ${CORE_BUILD_DIR}/${id}
                        CONFIGURE_COMMAND PKG_CONFIG_PATH=${DEPENDS_DIR}/lib/pkgconfig
                                          ${CMAKE_COMMAND} <SOURCE_DIR>
                                          -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
                                          -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                          -DBUILD_SHARED_LIBS=1
                                          -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                                          -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                                          -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                                          -DKODI_BUILD_DIR=${CMAKE_BINARY_DIR}
                                          -DBOOTSTRAP_IN_TREE=1
                        DEPENDS compileinfo
                        INSTALL_COMMAND "")
  elseif(BOOTSTRAP_DIR)
    externalproject_add(${id}
                        SOURCE_DIR ${BOOTSTRAP_DIR}/${CORE_BUILD_DIR}/${id}/src/${id}
                        PREFIX ${CORE_BUILD_DIR}/${id}
                        CONFIGURE_COMMAND PKG_CONFIG_PATH=${DEPENDS_DIR}/lib/pkgconfig
                                          ${CMAKE_COMMAND} <SOURCE_DIR>
                                          -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
                                          -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                          -DBUILD_SHARED_LIBS=1
                                          -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                                          -DKODI_BUILD_DIR=${CMAKE_BINARY_DIR}
                                          -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                                          -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                        INSTALL_COMMAND "")
  else()
    externalproject_add(${id}
                        GIT_REPOSITORY ${repo}
                        PREFIX ${OUTPUT_DIR}/${CORE_BUILD_DIR}/${id}
                        GIT_TAG ${hash}
                        UPDATE_COMMAND ${CMAKE_COMMAND}
                                       -DROOT_DIR=${OUTPUT_DIR}/${CORE_BUILD_DIR}/${id}/src/${id}
                                       -DAUTORECONF_EXECUTABLE=${AUTORECONF_EXECUTABLE}
                                       -P ${PROJECT_SOURCE_DIR}/installdata/bootstrap.cmake
                        CONFIGURE_COMMAND ""
                        BUILD_COMMAND ""
                        INSTALL_COMMAND "")
  endif()
  if(BUILDTOOLS)
    add_dependencies(${id} ${BUILDTOOLS})
  endif()
  add_dependencies(${id} kodi-platform)
endfunction()

function(add_addon_separate entry)
  string(REPLACE " " ";" entry "${entry}")
  list(GET entry 0 id)
  list(GET entry 1 repo)
  list(GET entry 2 hash)
  if("${id}" MATCHES "^visualization\\." OR
     "${id}" MATCHES "^screensavers?\\." OR
     "${id}" MATCHES "^audiodecoder\\." OR
     "${id}" MATCHES "^audioencoder\\." OR
     "${id}" MATCHES "^pvr\\." OR
     "${id}" MATCHES "^vfs\\.")
   add_git_cmake(${id} ${repo} ${hash})
  else()
    if(BOOTSTRAP_DIR)
      if("${id}" MATCHES "^skin.")
        list(APPEND SKINS "${BOOTSTRAP_DIR}/addons/${id}\;${BOOTSTRAP_DIR}")
      else()
        add_custom_command(OUTPUT ${OUTPUT_DIR}/addons/${id}
                           COMMAND cmake -E copy_directory ${BOOTSTRAP_DIR}}/addons/${id} ${OUTPUT_DIR}/addons/${id})
        list(APPEND SOURCES ${OUTPUT_DIR}/addons/${id})
        list(APPEND install_dirs ${OUTPUT_DIR}/addons/${id})
      endif()
    else()
      add_git_copy(${id} ${repo} ${hash} addons)
      list(APPEND install_dirs ${OUTPUT_DIR}/addons/${id})
      set(install_dirs ${install_dirs} PARENT_SCOPE)
      list(APPEND SOURCES ${OUTPUT_DIR}/addons/${id})
      if("${id}" MATCHES "^skin.")
        if(BOOTSTRAP_IN_TREE)
          list(APPEND EXTRA_XBT ${id})
        endif()
      endif()
    endif()
  endif()
  set(EXTRA_XBT ${EXTRA_XBT} PARENT_SCOPE)
  set(install_dirs ${install_dirs} PARENT_SCOPE)
  set(SOURCES ${SOURCES} PARENT_SCOPE)
  set(SKINS ${SKINS} PARENT_SCOPE)
endfunction()

function(add_addon_multi lines)
  list(LENGTH lines listlen)
  list(GET lines 0 base)
  if(listlen GREATER 1)
    list(GET lines 1 base2)
    list(REMOVE_ITEM lines ${base})
  endif()
  string(REPLACE " " ";" base ${base})
  list(GET base 0 id)
  list(GET base 1 repo)
  list(GET base 2 hash)
  if(listlen GREATER 1)
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
                           COMMAND ${CMAKE_COMMAND} -E copy_directory
                                   ${BOOTSTRAP_DIR}}/${entry} ${OUTPUT_DIR}/addons/${entry})
      else()
        add_custom_command(OUTPUT ${OUTPUT_DIR}/addons/${entry}
                           COMMAND ${CMAKE_COMMAND} -E copy_directory
                                   ${CMAKE_CURRENT_BINARY_DIR}/build/${id}/src/${id}/${entry}
                                   ${OUTPUT_DIR}/addons/${entry}
                           DEPENDS ${id})
      endif()
      list(APPEND SOURCES ${OUTPUT_DIR}/addons/${entry})
      list(APPEND install_dirs ${OUTPUT_DIR}/addons/${entry})
    endforeach()
  else()
    externalproject_add(${id}
                        GIT_REPOSITORY ${repo}
                        PREFIX build/${id}
                        GIT_TAG ${hash}
                        CONFIGURE_COMMAND ${CMAKE_COMMAND} ${PROJECT_SOURCE_DIR}/addon-build
                                          -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                          -DADDON_DIR=${PROJECT_BINARY_DIR}/build/${id}/src/${id}
                                          -DKODI_BUILD_DIR=${CMAKE_BINARY_DIR}
                                          -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/build
                                          -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                                          -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                                          -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                                          -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
                        INSTALL_COMMAND ""
                        DEPENDS kodi-platform)
    list(APPEND install_dirs ${PROJECT_BINARY_DIR}/build/${id}/src/${id}-build)
  endif()
  set(SOURCES ${SOURCES} PARENT_SCOPE)
  set(install_dirs ${install_dirs} PARENT_SCOPE)
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

file(GLOB optional_multi_input_files installdata/addons/multi/optional/common/*.txt)
file(GLOB multiopt2 installdata/addons/multi/optional/${CORE_SYSTEM_NAME}/*.txt)
list(APPEND optional_multi_input_files ${multiopt2})

file(GLOB_RECURSE lib_input_files installdata/lib/common/*.txt)
file(GLOB_RECURSE lib2 installdata/lib/${CORE_SYSTEM_NAME}/*.txt)
list(APPEND lib_input_files ${lib2})

file(GLOB_RECURSE optional_lib_input_files installdata/lib/optional/common/*.txt)
file(GLOB_RECURSE lib2 installdata/lib/optional/${CORE_SYSTEM_NAME}/*.txt)
list(APPEND optional_lib_input_files ${lib2})

if(NOT CORE_SOURCE_DIR)
  set(CORE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
endif()

if(NOT OUTPUT_DIR)
  set(OUTPUT_DIR ${CMAKE_BINARY_DIR})
endif()

# Libs hosted in a separate git
foreach(file ${lib_input_files})
  if(NOT file MATCHES CMakeLists.txt AND NOT file MATCHES install.txt)
    add_git_lib(${file})
  endif()
endforeach()

# Optional libs hosted in a separate git
foreach(file ${optional_lib_input_files})
  if(NOT file MATCHES CMakeLists.txt AND NOT file MATCHES install.txt)
    file(STRINGS ${file} entry)
    string(REPLACE " " ";" entry "${entry}")
    list(GET entry 0 id)
    list(GET entry 1 repo)
    list(GET entry 2 hash)
    list(GET entry 4 opts)
    list(GET entry 5 def)
    foreach(opt ${opts})
      option(ENABLE_${opt} "Enable ${id}?" ${def})
      if(ENABLE_${opt})
        add_git_lib(${file})
      endif()
    endforeach()
  endif()
endforeach()

# Add-ons hosted in a separate git
foreach(file ${separate_input_files})
  file(STRINGS ${file} entry)
  add_addon_separate(${entry})
endforeach()

# Optional add-ons hosted in a separate git
foreach(file ${optional_input_files})
  file(STRINGS ${file} entry)
  string(REPLACE " " ";" entry "${entry}")
  list(GET entry 0 id)
  list(GET entry 1 repo)
  list(GET entry 2 hash)
  list(GET entry 4 opts)
  list(GET entry 5 def)
  foreach(opt ${opts})
    option(ENABLE_${opt} "Enable ${id}?" ${def})
    if(ENABLE_${opt})
      add_addon_separate("${id} ${repo} ${hash}")
    endif()
  endforeach()
endforeach()

# Several add-ons host in subdirs of a common git
foreach(file ${multi_input_files})
  file(STRINGS ${file} lines)
  string(REPLACE "\n" ";" lines "${lines}")
  add_addon_multi("${lines}")
endforeach()

# Optional multi-addons;
# either copies subdirs of a repo
# or builds multiple addons in a git
foreach(file ${optional_multi_input_files})
  file(STRINGS ${file} lines2)
  string(REPLACE "\n" ";" lines "${lines2}")
  list(GET lines 0 base)
  string(REPLACE " " ";" base ${base})
  list(GET base 0 id)
  list(GET base 1 repo)
  list(GET base 2 hash)
  list(GET base 3 opt)
  list(GET base 4 def)
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
                     COMMAND ${AUTORECONF_EXECUTABLE} -vif WORKING_DIRECTORY ${CORE_SOURCE_DIR}/${dir})
  list(APPEND SOURCES ${CORE_SOURCE_DIR}/${dir}/config.h.in)
endforeach()

add_custom_target(bootstrap ALL SOURCES ${SOURCES})

if(BOOTSTRAP_IN_TREE OR BOOTSTRAP_DIR)
  set(install_dirs ${install_dirs} PARENT_SCOPE)
  set(EXTRA_XBT ${EXTRA_XBT} PARENT_SCOPE)
  set(SKINS ${SKINS} PARENT_SCOPE)
endif()

if(NOT BOOTSTRAP_DIR)
  # Build helpers
  add_executable(JsonSchemaBuilder ${CORE_SOURCE_DIR}/tools/depends/native/JsonSchemaBuilder/src/JsonSchemaBuilder.cpp)
  add_subdirectory(${CORE_SOURCE_DIR}/tools/depends/native/TexturePacker texturepacker)
endif()
