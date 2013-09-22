project(addon-builder)

include(ExternalProject)
cmake_minimum_required(VERSION 2.8)

function(add_git_cmake id repo hash)
  externalproject_add(${id}
                      GIT_REPOSITORY ${repo}
                      GIT_TAG ${hash}
                      PREFIX build/${id}
                      CONFIGURE_COMMAND PKG_CONFIG_PATH=${DEPENDS_DIR}/lib/pkgconfig
                                        ${CMAKE_COMMAND} <SOURCE_DIR>
                                        -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
                                        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                        -DBUILD_SHARED_LIBS=1
                                        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                                        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                                        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                                        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
                                        -DKODI_BUILD_DIR=${KODI_BUILD_DIR}
                                        -DBOOTSTRAP_IN_TREE=1
                                        -DPACKAGE_ZIP=1
                      INSTALL_COMMAND "")

  # TODO: Unix specific
  add_custom_target(package-${id}
                    ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/build/${id}/src/${id}-build --target addon-package
                    COMMAND mv ${CMAKE_BINARY_DIR}/build/${id}/src/${id}-build/${id}-*.zip ${CMAKE_BINARY_DIR}
                    DEPENDS ${id})
endfunction()

function(add_addon_separate entry)
  string(REPLACE " " ";" entry "${entry}")
  list(GET entry 0 id)
  list(GET entry 1 repo)
  list(GET entry 2 hash)
  list(LENGTH entry numentries)
  if(numentries GREATER 3)
    list(GET entry 4 opt)
    if(NOT ENABLE_${opt})
      return()
    endif()
  endif()
  add_git_cmake(${id} ${repo} ${hash})
  list(APPEND targets ${id})
  set(targets ${targets} PARENT_SCOPE)
endfunction()

if(NOT KODI_BUILD_DIR)
  set(KODI_BUILD_DIR ${CMAKE_BINARY_DIR})
endif()

file(GLOB_RECURSE separate_input_files ${ADDON_DIR}/*.txt)

# Add-ons hosted in a separate git
foreach(file ${separate_input_files})
  file(STRINGS ${file} entry)
  add_addon_separate(${entry})
endforeach()

# Package targets
add_custom_target(addon-package)
foreach(addon ${targets})
  add_dependencies(addon-package package-${addon})
endforeach()
