# This script holds the main functions used to construct the build system

# include system specific macros
include(${PROJECT_SOURCE_DIR}/scripts/${CORE_SYSTEM_NAME}/macros.cmake)

# Add a library, optionally as a dependency of the main application
# Arguments:
#   name name of the library to add
#   if another argument is given, library is not added to main depends
# Implicit arguments:
#   SOURCES the sources of the library
# On return:
#   Library will be built, optionally added to ${core_DEPENDS}
function(core_add_library name)
  add_library(${name} STATIC ${SOURCES})
  set_target_properties(${name} PROPERTIES PREFIX "")
  if("${ARGN}" STREQUAL "")
    set(core_DEPENDS ${name} ${core_DEPENDS} CACHE STRING "" FORCE)
  endif()
endfunction()

# Add a test library, and add sources to list for gtest integration macros
function(core_add_test_library name)
  if(NOT ENABLE_TESTS)
    return()
  endif()
  core_add_library(${name} 1)
  foreach(src ${SOURCES})
    set(test_sources ${CMAKE_CURRENT_SOURCE_DIR}/${src} ${test_sources} CACHE STRING "" FORCE)
  endforeach()
  set(test_archives ${test_archives} ${name} CACHE STRING "" FORCE)
endfunction()

# Add a data file to installation list with a mirror in build tree
# Arguments:
#   file     full path to file to mirror
#   relative the relative base of file path in the build/install tree
# Implicit arguments:
#   CORE_SOURCE_DIR - root of source tree
# On return:
#   file is added to ${install_data} and mirrored in build tree
function(copy_file_to_buildtree file relative)
  string(REPLACE "\(" "\\(" filename ${file})
  string(REPLACE "\)" "\\)" file2 ${filename})
  string(REPLACE "${relative}/" "" file3 ${file2})

  if(NOT TARGET export-files)
    add_custom_target(export-files ALL COMMENT "Copying files into build tree")
  endif()
  if(NOT ${CORE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
    if(VERBOSE)
      message(STATUS "copy_file_to_buildtree - copying file: ${file2} -> ${CMAKE_CURRENT_BINARY_DIR}/${file3}")
    endif()
    add_custom_command(TARGET export-files COMMAND ${CMAKE_COMMAND} -E copy_if_different "${file2}" "${CMAKE_CURRENT_BINARY_DIR}/${file3}")
  endif()
  list(APPEND install_data ${file3})
  set(install_data ${install_data} PARENT_SCOPE)
endfunction()

# add data files to installation list with a mirror in build tree.
# reads list of files to install from a given list of text files.
# Arguments:
#   pattern globbing pattern for text files to read
# Implicit arguments:
#   CORE_SOURCE_DIR - root of source tree
# On return:
#   files are added to ${install_data} and mirrored in build tree
function(copy_files_from_filelist_to_buildtree pattern)
  foreach(arg ${ARGN})
    list(APPEND pattern ${arg})
  endforeach()
  # copies files listed in text files to the buildtree
  # Input: [glob pattern: filepattern]
  list(SORT pattern)
  if(VERBOSE)
    message(STATUS "copy_files_from_filelist_to_buildtree - got pattern: ${pattern}")
  endif()
  foreach(pat ${pattern})
    file(GLOB filenames ${pat})
    foreach(filename ${filenames})
      string(STRIP ${filename} filename)
      core_file_read_filtered(fstrings ${filename})
      foreach(dir ${fstrings})
        file(GLOB_RECURSE files RELATIVE ${CORE_SOURCE_DIR} ${CORE_SOURCE_DIR}/${dir})
        foreach(file ${files})
          copy_file_to_buildtree(${CORE_SOURCE_DIR}/${file} ${CORE_SOURCE_DIR})
        endforeach()
      endforeach()
    endforeach()
  endforeach()
  set(install_data ${install_data} PARENT_SCOPE)
endfunction()

# helper macro to set modified variables in parent scope
macro(export_dep)
  set(SYSTEM_INCLUDES ${SYSTEM_INCLUDES} PARENT_SCOPE)
  set(DEPLIBS ${DEPLIBS} PARENT_SCOPE)
  set(DEP_DEFINES ${DEP_DEFINES} PARENT_SCOPE)
  set(${depup}_FOUND ${${depup}_FOUND} PARENT_SCOPE)
  mark_as_advanced(${depup}_LIBRARIES)
endmacro()

# add a required dependency of main application
# Arguments:
#   dep name of find rule for dependency, used uppercased for variable prefix
# On return:
#   dependency added to ${SYSTEM_INCLUDES}, ${DEPLIBS} and ${DEP_DEFINES}
function(core_require_dep dep)
  find_package(${dep} REQUIRED)
  string(TOUPPER ${dep} depup)
  list(APPEND SYSTEM_INCLUDES ${${depup}_INCLUDE_DIRS})
  list(APPEND DEPLIBS ${${depup}_LIBRARIES})
  list(APPEND DEP_DEFINES ${${depup}_DEFINITIONS})
  export_dep()
endfunction()

# add a required dyloaded dependency of main application
# Arguments:
#   dep name of find rule for dependency, used uppercased for variable prefix
# On return:
#   dependency added to ${SYSTEM_INCLUDES}, ${dep}_SONAME is set up
function(core_require_dyload_dep dep)
  find_package(${dep} REQUIRED)
  string(TOUPPER ${dep} depup)
  list(APPEND SYSTEM_INCLUDES ${${depup}_INCLUDE_DIRS})
  find_soname(${depup})
  export_dep()
  set(${depup}_SONAME ${${depup}_SONAME} PARENT_SCOPE)
endfunction()

# helper macro for optional deps
macro(setup_enable_switch)
  string(TOUPPER ${dep} depup)
  if (ARGV1)
    set(enable_switch ${ARGV1})
  else()
    set(enable_switch ENABLE_${depup})
  endif()
endmacro()

# add an optional dependency of main application
# Arguments:
#   dep name of find rule for dependency, used uppercased for variable prefix
# On return:
#   dependency optionally added to ${SYSTEM_INCLUDES}, ${DEPLIBS} and ${DEP_DEFINES}
function(core_optional_dep dep)
  setup_enable_switch()
  if(${enable_switch})
    find_package(${dep})
    if(${depup}_FOUND)
      list(APPEND SYSTEM_INCLUDES ${${depup}_INCLUDE_DIRS})
      list(APPEND DEPLIBS ${${depup}_LIBRARIES})
      list(APPEND DEP_DEFINES ${${depup}_DEFINITIONS})
      set(final_message ${final_message} "${depup} enabled: Yes" PARENT_SCOPE)
      export_dep()
    else()
      set(final_message ${final_message} "${depup} enabled: No" PARENT_SCOPE)
    endif()
  endif()
endfunction()

# add an optional dyloaded dependency of main application
# Arguments:
#   dep name of find rule for dependency, used uppercased for variable prefix
# On return:
#   dependency optionally added to ${SYSTEM_INCLUDES}, ${DEP_DEFINES}, ${dep}_SONAME is set up
function(core_optional_dyload_dep dep)
  setup_enable_switch()
  if(${enable_switch})
    find_package(${dep})
    if(${depup}_FOUND)
      list(APPEND SYSTEM_INCLUDES ${${depup}_INCLUDE_DIRS})
      find_soname(${depup})
      list(APPEND DEP_DEFINES ${${depup}_DEFINITIONS})
      set(final_message ${final_message} "${depup} enabled: Yes" PARENT_SCOPE)
      export_dep()
      set(${depup}_SONAME ${${depup}_SONAME} PARENT_SCOPE)
    endif()
  endif()
endfunction()

function(core_file_read_filtered result filepattern)
  # Reads STRINGS from text files
  #  with comments filtered out
  # Result: [list: result]
  # Input:  [glob pattern: filepattern]
  file(GLOB filenames ${filepattern})
  list(SORT filenames)
  foreach(filename ${filenames})
    if(VERBOSE)
      message(STATUS "core_file_read_filtered - filename: ${filename}")
    endif()
    file(STRINGS ${filename} fstrings REGEX "^[^#//]")
    foreach(fstring ${fstrings})
      string(REGEX REPLACE "^(.*)#(.*)" "\\1" fstring ${fstring})
      string(REGEX REPLACE "//.*" "" fstring ${fstring})
      string(STRIP ${fstring} fstring)
      list(APPEND filename_strings ${fstring})
    endforeach()
  endforeach()
  set(${result} ${filename_strings} PARENT_SCOPE)
endfunction()

function(core_add_subdirs_from_filelist files)
  # Adds subdirectories from a sorted list of files
  # Input: [list: filenames] [bool: sort]
  foreach(arg ${ARGN})
    list(APPEND files ${arg})
  endforeach()
  list(SORT files)
  if(VERBOSE)
    message(STATUS "core_add_subdirs_from_filelist - got pattern: ${files}")
  endif()
  foreach(filename ${files})
    string(STRIP ${filename} filename)
    core_file_read_filtered(fstrings ${filename})
    foreach(subdir ${fstrings})
      STRING_SPLIT(subdir " " ${subdir})
      list(GET subdir  0 subdir_src)
      list(GET subdir -1 subdir_dest)
      if(VERBOSE)
        message(STATUS "  core_add_subdirs_from_filelist - adding subdir: ${CORE_SOURCE_DIR}${subdir_src} -> ${CORE_BUILD_DIR}/${subdir_dest}")
      endif()
      add_subdirectory(${CORE_SOURCE_DIR}/${subdir_src} ${CORE_BUILD_DIR}/${subdir_dest})
    endforeach()
  endforeach()
endfunction()

macro(core_add_optional_subdirs_from_filelist pattern)
  # Adds subdirectories from text files
  #  if the option(s) in the 3rd field are enabled
  # Input: [glob pattern: filepattern]
  foreach(arg ${ARGN})
    list(APPEND pattern ${arg})
  endforeach()
  foreach(elem ${pattern})
    string(STRIP ${elem} elem)
    list(APPEND filepattern ${elem})
  endforeach()

  file(GLOB filenames ${filepattern})
  list(SORT filenames)
  if(VERBOSE)
    message(STATUS "core_add_optional_subdirs_from_filelist - got pattern: ${filenames}")
  endif()

  foreach(filename ${filenames})
    if(VERBOSE)
      message(STATUS "core_add_optional_subdirs_from_filelist - reading file: ${filename}")
    endif()
    file(STRINGS ${filename} fstrings REGEX "^[^#//]")
    foreach(line ${fstrings})
      string(REPLACE " " ";" line "${line}")
      list(GET line 0 subdir_src)
      list(GET line 1 subdir_dest)
      list(GET line 3 opts)
      foreach(opt ${opts})
        if(ENABLE_${opt})
          if(VERBOSE)
            message(STATUS "  core_add_optional_subdirs_from_filelist - adding subdir: ${CORE_SOURCE_DIR}${subdir_src} -> ${CORE_BUILD_DIR}/${subdir_dest}")
          endif()
          add_subdirectory(${CORE_SOURCE_DIR}/${subdir_src} ${CORE_BUILD_DIR}/${subdir_dest})
         else()
            if(VERBOSE)
              message(STATUS "  core_add_optional_subdirs_from_filelist: OPTION ${opt} not enabled for ${subdir_src}, skipping subdir")
            endif()
        endif()
      endforeach()
    endforeach()
  endforeach()
endmacro()
