function(xbmc_add_library name)
  add_library(${name} STATIC ${SOURCES})
  set_target_properties(${name} PROPERTIES PREFIX "")
  if("${ARGN}" STREQUAL "")
    set(xbmc-bin_DEPENDS ${name} ${xbmc-bin_DEPENDS} CACHE STRING "" FORCE)
  endif()
endfunction()

function(xbmc_link_library lib wraplib)
  set(export -Wl,--unresolved-symbols=ignore-all
             `cat ${CMAKE_BINARY_DIR}/dllloader.dir/exports/wrapper.def`
             ${CMAKE_BINARY_DIR}/dllloader.dir/exports/CMakeFiles/wrapper.dir/wrapper.c.o)
  set(check_arg "")
  if(TARGET ${lib})
    set(target ${lib})
    set(link_lib ${CMAKE_BINARY_DIR}/${lib}.dir/${lib}.a)
    set(check_arg ${ARGV2})
    set(data_arg  ${ARGV3})
  else()
    set(target ${ARGV2})
    set(link_lib ${lib})
    set(check_arg ${ARGV3})
    set(data_arg ${ARGV4})
  endif()
  if("${check_arg}" STREQUAL "export")
    set(export ${export} 
        -Wl,--version-script=${ARGV3})
  elseif("${check_arg}" STREQUAL "nowrap")
    set(export ${data_arg})
  elseif("${check_arg}" STREQUAL "extras")
    list(APPEND export ${data_arg})
  endif()
  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/system/${wraplib}-${ARCH}${CMAKE_SHARED_MODULE_SUFFIX}
                     COMMAND ${CMAKE_C_COMPILER}
                     ARGS    -Wl,--whole-archive
                             ${link_lib}
                             -Wl,--no-whole-archive -lm
                             -shared -o ${CMAKE_BINARY_DIR}/system/${wraplib}-${ARCH}${CMAKE_SHARED_MODULE_SUFFIX}
                             ${export}
                     DEPENDS ${target} wrapper.def wrapper)
  list(APPEND WRAP_FILES ${CMAKE_BINARY_DIR}/system/${wraplib}-${ARCH}${CMAKE_SHARED_MODULE_SUFFIX})
  set(WRAP_FILES ${WRAP_FILES} PARENT_SCOPE)
  get_filename_component(dir ${wraplib} PATH)
  install(PROGRAMS ${CMAKE_BINARY_DIR}/system/${wraplib}-${ARCH}${CMAKE_SHARED_MODULE_SUFFIX}
          DESTINATION lib/xbmc/system/${dir})
endfunction()

function(copy_file_to_buildtree file)
  if (NOT TARGET export-files)
    add_custom_target(export-files ALL COMMENT "Copying files into build tree")
  endif (NOT TARGET export-files)
  string(REPLACE "\(" "\\(" filename ${file})
  string(REPLACE "\)" "\\)" file2 ${filename})
  add_custom_command(TARGET export-files COMMAND ${CMAKE_COMMAND} -E copy_if_different "${XBMC_SOURCE_DIR}/${file2}" "${CMAKE_CURRENT_BINARY_DIR}/${file2}")
  get_filename_component(dir ${file2} PATH)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${file2}
          DESTINATION share/xbmc/${dir})
endfunction()

function(copy_skin_to_buildtree skin)
  file(GLOB_RECURSE FILES RELATIVE ${XBMC_SOURCE_DIR} ${XBMC_SOURCE_DIR}/${skin}/*)
  file(GLOB_RECURSE MEDIA_FILES RELATIVE ${XBMC_SOURCE_DIR} ${XBMC_SOURCE_DIR}/${skin}/media/*)
  list(REMOVE_ITEM FILES ${MEDIA_FILES})
  file(GLOB_RECURSE MEDIA_FILES ${XBMC_SOURCE_DIR}/${skin}/media/*)
  foreach(file ${FILES})
    copy_file_to_buildtree(${file})
  endforeach()
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${skin}/media)
  add_custom_command(OUTPUT  ${skin}/media/Textures.xbt
                     COMMAND ${CMAKE_BINARY_DIR}/texturepacker.dir/TexturePacker
                     ARGS    -input ${XBMC_SOURCE_DIR}/${skin}/media
                             -output ${CMAKE_BINARY_DIR}/${skin}/media/Textures.xbt
                             -dupecheck
                     DEPENDS ${MEDIA_FILES} TexturePacker)
  list(APPEND XBT_FILES ${skin}/media/Textures.xbt)
  install(FILES ${CMAKE_BINARY_DIR}/${skin}/media/Textures.xbt
          DESTINATION share/xbmc/${skin}/media)
                
  set(XBT_FILES ${XBT_FILES} PARENT_SCOPE)
endfunction()

function(xbmc_find_soname lib)
  if(ARGV1)
    set(liblow ${ARGV1})
  else()
    string(TOLOWER ${lib} liblow)
  endif()
  if(${lib}_LDFLAGS)
    set(link_lib "${${lib}_LDFLAGS}")
  else()
    if(IS_ABSOLUTE "${${lib}_LIBRARIES}")
      set(link_lib "${${lib}_LIBRARIES}")
    else()
      set(link_lib -l${${lib}_LIBRARIES})
    endif()
  endif()
  execute_process(COMMAND ${CMAKE_C_COMPILER} -nostdlib -o /dev/null -Wl,-M ${link_lib} 
                  COMMAND grep LOAD.*${liblow}
                  ERROR_QUIET
                  OUTPUT_VARIABLE ${lib}_FILENAME)
  string(REPLACE "LOAD " "" ${lib}_FILENAME "${${lib}_FILENAME}")
  string(STRIP "${${lib}_FILENAME}" ${lib}_FILENAME)
  if(${lib}_FILENAME)
    execute_process(COMMAND objdump -p ${${lib}_FILENAME}
                    COMMAND grep SONAME.*${liblow}
                    ERROR_QUIET
                    OUTPUT_VARIABLE ${lib}_SONAME)
    string(REPLACE "SONAME " "" ${lib}_SONAME ${${lib}_SONAME})
    string(STRIP ${${lib}_SONAME} ${lib}_SONAME)
    message(STATUS "${lib} soname: ${${lib}_SONAME}")
    set(${lib}_SONAME ${${lib}_SONAME} PARENT_SCOPE)
  endif()
endfunction()

macro(export_dep)
  set(SYSTEM_INCLUDES ${SYSTEM_INCLUDES} PARENT_SCOPE)
  set(DEPLIBS ${DEPLIBS} PARENT_SCOPE)
  set(DEP_DEFINES ${DEP_DEFINES} PARENT_SCOPE)
  set(${depup}_FOUND ${${depup}_FOUND} PARENT_SCOPE)
endmacro()

function(xbmc_require_dep dep)
  find_package(${dep} REQUIRED)
  string(TOUPPER ${dep} depup)
  list(APPEND SYSTEM_INCLUDES ${${depup}_INCLUDE_DIRS})
  list(APPEND DEPLIBS ${${depup}_LIBRARIES})
  list(APPEND DEP_DEFINES ${${depup}_DEFINITIONS})
  export_dep()
endfunction()

function(xbmc_require_dyload_dep dep)
  find_package(${dep} REQUIRED)
  string(TOUPPER ${dep} depup)
  list(APPEND SYSTEM_INCLUDES ${${depup}_INCLUDE_DIRS})
  xbmc_find_soname(${depup})
  export_dep()
  set(${depup}_SONAME ${${depup}_SONAME} PARENT_SCOPE)
endfunction()

macro(setup_enable_switch)
  string(TOUPPER ${dep} depup)
  if (ARGV1)
    set(enable_switch ${ARGV1})
  else()
    set(enable_switch ENABLE_${depup})
  endif()
endmacro()

function(xbmc_optional_dep dep)
  setup_enable_switch()
  if(${enable_switch})
    find_package(${dep})
    if(${depup}_FOUND)
      list(APPEND SYSTEM_INCLUDES ${${depup}_INCLUDE_DIRS})
      list(APPEND DEPLIBS ${${depup}_LIBRARIES})
      list(APPEND DEP_DEFINES ${${depup}_DEFINITIONS})
      export_dep()
    endif()
  endif()
endfunction()

function(xbmc_optional_dyload_dep dep)
  setup_enable_switch()
  if(${enable_switch})
    find_package(${dep})
    if(${depup}_FOUND)
      list(APPEND SYSTEM_INCLUDES ${${depup}_INCLUDE_DIRS})
      xbmc_find_soname(${depup})
      list(APPEND DEP_DEFINES ${${depup}_DEFINITIONS})
      export_dep()
      set(${depup}_SONAME ${${depup}_SONAME} PARENT_SCOPE)
    endif()
  endif()
endfunction()

function(install_rename file dest)
  get_filename_component(dir ${dest} PATH)
  install(CODE "execute_process(COMMAND install -d $ENV{DESTDIR}${dir}
                                COMMAND install -m0644 ${file} $ENV{DESTDIR}${dest})")
endfunction()
