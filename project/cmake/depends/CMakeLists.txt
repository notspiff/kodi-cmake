project(xbmc-depends)

function(add_internal id url inputfile)
  externalproject_add(${id}
                      URL ${url}
                      PREFIX build/${id}
                      CONFIGURE_COMMAND ""
                      BUILD_COMMAND ""
                      INSTALL_COMMAND ${CMAKE_COMMAND}
                                      -DINPUTDIR=${CMAKE_BINARY_DIR}/build/${id}/src/${id}
				      -DINPUTFILE=${inputfile}
                                      -DDESTDIR=${OUTPUT_DIR}
                                      -P ${PROJECT_SOURCE_DIR}/install.cmake)
endfunction()

function(add_7zip id url inputfile)
  externalproject_add(${id}
                      PREFIX build/${id}
                      DOWNLOAD_COMMAND ${CMAKE_COMMAND}
                                       -D7ZIP_EXECUTABLE=${7ZIP_EXECUTABLE}
                                       -DURL=${url}
                                       -DDEST=${CMAKE_BINARY_DIR}/build/${id}/src/${id}
                                      -DDESTDIR=${CMAKE_BINARY_DIR}/build/${id}/src/${id}-build
                                      -P ${PROJECT_SOURCE_DIR}/extract-7z.cmake
                      CONFIGURE_COMMAND ""
                      BUILD_COMMAND ""
                      INSTALL_COMMAND ${CMAKE_COMMAND}
                                      -DINPUTDIR=${CMAKE_BINARY_DIR}/build/${id}/src/${id}-build/
		  	 	      -DINPUTFILE=${inputfile}
                                      -DDESTDIR=${OUTPUT_DIR}
                                      -P ${PROJECT_SOURCE_DIR}/install.cmake)
endfunction()

function(add_direct id url inputfile)
  externalproject_add(${id}
                      PREFIX build/${id}
                      DOWNLOAD_COMMAND ${CMAKE_COMMAND}
                                       -DURL=${url}
                                       -DDEST=${CMAKE_BINARY_DIR}/build/${id}/src/${id}
                                      -P ${PROJECT_SOURCE_DIR}/extract-direct.cmake
                      CONFIGURE_COMMAND ""
                      BUILD_COMMAND ""
                      INSTALL_COMMAND ${CMAKE_COMMAND}
                                      -DINPUTDIR=${CMAKE_BINARY_DIR}/build/${id}/src/${id}/
		  	 	      -DINPUTFILE=${inputfile}
                                      -DDESTDIR=${OUTPUT_DIR}
                                      -P ${PROJECT_SOURCE_DIR}/install.cmake)
endfunction()

cmake_minimum_required(VERSION 2.8)

if(NOT CORE_SYSTEM_NAME)
  string(TOLOWER ${CMAKE_SYSTEM_NAME} CORE_SYSTEM_NAME)
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(ExternalProject)

if(NOT OUTPUT_DIR)
  set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/depends)
endif()
list(APPEND CMAKE_PREFIX_PATH ${OUTPUT_DIR})

file(GLOB_RECURSE cmake_input_files ${CORE_SYSTEM_NAME}/*.txt)
file(GLOB_RECURSE cmake_input_files2 common/*.txt)
list(APPEND cmake_input_files ${cmake_input_files2})
if(BOOTSTRAP_DIR)
  file(GLOB bootstrap_dirs ${BOOTSTRAP_DIR}/addons/compile/*)
  foreach(dir ${bootstrap_dirs})
    get_filename_component(id ${dir} NAME)
    file(GLOB_RECURSE cmake_input_files2 ${dir}/src/${id}/depends/${CORE_SYSTEM_NAME}/*.txt)
    file(GLOB_RECURSE cmake_input_files2 ${dir}/src/${id}/depends/common/*.txt)
    foreach(file ${cmake_input_files2})
      list(APPEND cmake_input_files ${file})
    endforeach()
  endforeach()
endif()

foreach(file ${cmake_input_files})
  if(NOT (file MATCHES CMakeLists.txt OR
  	  file MATCHES install.txt OR
          file MATCHES flags.txt))
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
    file(REMOVE ${CMAKE_BINARY_DIR}/build/${id}/tmp/patch.cmake)
    if(EXISTS ${dir}/CMakeLists.txt)
      file(APPEND ${CMAKE_BINARY_DIR}/build/${id}/tmp/patch.cmake
           "file(COPY ${dir}/CMakeLists.txt DESTINATION ${CMAKE_BINARY_DIR}/build/${id}/src/${id})\n")
      set(PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/build/${id}/tmp/patch.cmake)
    endif()
    file(GLOB patches ${dir}/*.patch)
    list(SORT patches)
    foreach(patch ${patches})
      set(PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/build/${id}/tmp/patch.cmake)
      file(APPEND ${CMAKE_BINARY_DIR}/build/${id}/tmp/patch.cmake
           "execute_process(COMMAND patch -p1 -i ${patch})\n")
    endforeach()
    if(EXISTS ${dir}/install.txt)
      set(INSTALL_COMMAND INSTALL_COMMAND ${CMAKE_COMMAND}
                                          -DINPUTDIR=${CMAKE_BINARY_DIR}/build/${id}/src/${id}-build/
		  	 	          -DINPUTFILE=${dir}/install.txt
                                          -DDESTDIR=${OUTPUT_DIR}
					  "${extraflags}"
                                          -P ${PROJECT_SOURCE_DIR}/install.cmake)
    else()
      set(INSTALL_COMMAND "")
    endif()
    if(NOT TARGET ${id})
      externalproject_add(${id} URL ${url} 
                          PREFIX build/${id}
                          CONFIGURE_COMMAND PKG_CONFIG_PATH=${OUTPUT_DIR}/lib/pkgconfig 
                                            ${CMAKE_COMMAND} 
                                            ${CMAKE_BINARY_DIR}/build/${id}/src/${id}
                                            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                            -DOUTPUT_DIR=${OUTPUT_DIR}
                                            -DCMAKE_PREFIX_PATH=${OUTPUT_DIR}
                                            -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}
                                            ${extraflags}
                          PATCH_COMMAND ${PATCH_COMMAND}
                          "${INSTALL_COMMAND}")
      list(LENGTH def deflength)
      if(deflength GREATER 2)
        list(GET def 2 deps)
        add_dependencies(${id} ${deps})
      endif()
    endif()
  endif()
endforeach()
