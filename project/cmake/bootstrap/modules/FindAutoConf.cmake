if(NOT BOOTSTRAP_IN_TREE) # fixme
  file(STRINGS ${PROJECT_SOURCE_DIR}/installdata/buildtools/autoconf/autoconf.txt def)
  set(dir ${PROJECT_SOURCE_DIR}/installdata/buildtools/autoconf)
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
                                        -DDESTDIR=${CMAKE_BINARY_DIR}
                                        "${extraflags}"
                                        -P ${PROJECT_SOURCE_DIR}/install.cmake)
  else()
    set(INSTALL_COMMAND "")
  endif()
  if(NOT TARGET ${id})
    externalproject_add(${id} URL ${url} 
                        PREFIX build/${id}
                        CONFIGURE_COMMAND PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/lib/pkgconfig
                                          ${CMAKE_COMMAND} 
                                          ${CMAKE_BINARY_DIR}/build/${id}/src/${id}
                                          -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                          -DOUTPUT_DIR=${CMAKE_BINARY_DIR}
                                          -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}
                                          -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}
                                          ${extraflags}
                        PATCH_COMMAND ${PATCH_COMMAND}
                        "${INSTALL_COMMAND}")
    list(LENGTH def deflength)
    if(deflength GREATER 2)
      list(GET def 2 deps)
      add_dependencies(${id} ${deps})
    endif()
  endif()

  set(AUTORECONF_EXECUTABLE ${CMAKE_BINARY_DIR}/bin/autoreconf)
  list(APPEND BUILDTOOLS autoconf)
else()
  find_program(AUTORECONF_EXECUTABLE NAMES autoreconf)
endif()

find_package_handle_standard_args(AutoConf DEFAULT_MSG AUTORECONF_EXECUTABLE)

mark_as_advanced(AUTORECONF_EXECUTABLE)
