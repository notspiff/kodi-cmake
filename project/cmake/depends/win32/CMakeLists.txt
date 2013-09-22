project(xbmc-win32-depends)


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

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(ExternalProject)

if(NOT OUTPUT_DIR)
  set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/depends)
endif()
list(APPEND CMAKE_PREFIX_PATH ${OUTPUT_DIR})

file(GLOB_RECURSE cmake_input_files cmake/*.txt)
foreach(file ${cmake_input_files})
  if(NOT (file MATCHES CMakeLists.txt OR
  	  file MATCHES install.txt OR
          file MATCHES flags.txt))
    file(STRINGS ${file} def)
    get_filename_component(dir ${file} PATH)
    string(REPLACE " " ";" def ${def})
    list(GET def 0 id)
    list(GET def 1 url)
    if(EXISTS ${dir}/flags.txt})
      file(STRINGS ${dir}/flags.txt extraflags)
    endif()
    if(EXISTS ${dir}/CMakeLists.txt)
      set(PATCH_COMMAND ${CMAKE_COMMAND} -E copy
                        ${dir}/CMakeLists.txt
                        ${CMAKE_BINARY_DIR}/build/${id}/src/${id})
    else()
      set(PATCH_COMMAND "")
    endif()
    if(EXISTS ${dir}/install.txt)
      set(INSTALL_COMMAND INSTALL_COMMAND ${CMAKE_COMMAND}
                                          -DINPUTDIR=${CMAKE_BINARY_DIR}/build/${id}/src/${id}-build/
		  	 	          -DINPUTFILE=${dir}/install.txt
                                          -DDESTDIR=${OUTPUT_DIR}
					  -DENABLE_STATIC=1
					  "${extraflags}"
                                          -P ${PROJECT_SOURCE_DIR}/install.cmake)
    else()
      set(INSTALL_COMMAND "")
    endif()
    externalproject_add(${id} URL ${url} 
                        PREFIX build/${id}
                        CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                   -DOUTPUT_DIR=${OUTPUT_DIR}
				   -DENABLE_STATIC=1
				   -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}
				   "${extraflags}"
                        PATCH_COMMAND ${PATCH_COMMAND}
			"${INSTALL_COMMAND}")
    list(LENGTH def deflength)
    if(deflength GREATER 2)
      list(GET def 2 deps)
      add_dependencies(${id} ${deps})
    endif()
  endif()
endforeach()

find_package(7Zip REQUIRED)

file(GLOB_RECURSE download_input_files download/*.txt)
foreach(file ${download_input_files})
  if(NOT file MATCHES install.txt)
    file(STRINGS ${file} def)
    get_filename_component(dir ${file} PATH)
    string(REPLACE " " ";" def ${def})
    list(GET def 0 id)
    list(GET def 1 url)
    if(url MATCHES .7z OR url MATCHES .lzma)
      add_7zip(${id} ${url} ${dir}/install.txt)
    elseif(url MATCHES .exe)
      add_direct(${id} ${url} ${dir}/install.txt)
    else()
      add_internal(${id} ${url} ${dir}/install.txt)
    endif()
  endif()
endforeach()
