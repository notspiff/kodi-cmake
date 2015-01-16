# This script holds macros which are project specific

# Pack a skin xbt file
# Arguments:
#   input  input directory to pack
#   output ouput xbt file
# On return:
#   xbt is added to ${XBT_FILES}
function(pack_xbt input output)
  if(BOOTSTRAP_DIR)
    set(TexturePacker ${BOOTSTRAP_DIR}/texturepacker/TexturePacker)
  else()
    set(TexturePacker ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/bootstrap/texturepacker/TexturePacker)
  endif()
  file(GLOB_RECURSE MEDIA_FILES ${input}/*)
  get_filename_component(dir ${output} PATH)
  add_custom_command(OUTPUT  ${output}
                     COMMAND ${CMAKE_COMMAND} -E make_directory ${dir}
                     COMMAND ${TexturePacker}
                     ARGS    -input ${input}
                             -output ${output}
                             -dupecheck
                     DEPENDS ${MEDIA_FILES})
  list(APPEND XBT_FILES ${output})
  set(XBT_FILES ${XBT_FILES} PARENT_SCOPE)
endfunction()

# Add a skin to installation list, mirroring it in build tree, packing textures
# Arguments:
#   skin     skin directory
#   relative relative base path in build tree
# On return:
#   xbt is added to ${XBT_FILES}, data added to ${install_data}, mirror in build tree
function(copy_skin_to_buildtree skin relative)
  file(GLOB_RECURSE FILES ${skin}/*)
  file(GLOB_RECURSE MEDIA_FILES ${skin}/media/*)
  list(REMOVE_ITEM FILES ${MEDIA_FILES})
  foreach(file ${FILES})
    copy_file_to_buildtree(${file} ${relative})
  endforeach()
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${dest}/media)
  string(REPLACE "${relative}/" "" dest ${skin})
  pack_xbt(${skin}/media
           ${CMAKE_BINARY_DIR}/${dest}/media/Textures.xbt
           ${CMAKE_BINARY_DIR})
                
  set(XBT_FILES ${XBT_FILES} PARENT_SCOPE)
  set(install_data ${install_data} PARENT_SCOPE)
endfunction()

# Prepare add-on build environment
# On return:
#   build tree is ready for building add-ons
function(prepare_addon_env)
  set(prefix ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR})
  file(COPY ${addon_bindings} DESTINATION ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/include/kodi)
  # Generate files with deprecation warnings
  foreach(binding ${addon_bindings})
    get_filename_component(file ${binding} NAME)
    file(WRITE ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/include/xbmc/${file}
"#pragma once
#warning \"Including xbmc/${file} is deprecated, use kodi/${file}\"
#include \"kodi/${file}\"
")
  endforeach()
endfunction()

# Get GTest tests as CMake tests.
# Copied from FindGTest.cmake 
# Thanks to Daniel Blezek <blezek@gmail.com> for the GTEST_ADD_TESTS code
function(GTEST_ADD_TESTS executable extra_args)
    if(NOT ARGN)
        message(FATAL_ERROR "Missing ARGN: Read the documentation for GTEST_ADD_TESTS")
    endif()
    foreach(source ${ARGN})
        file(READ "${source}" contents)
        string(REGEX MATCHALL "TEST_?[F]?\\(([A-Za-z_0-9 ,]+)\\)" found_tests ${contents})
        foreach(hit ${found_tests})
            string(REGEX REPLACE ".*\\( *([A-Za-z_0-9]+), *([A-Za-z_0-9]+) *\\).*" "\\1.\\2" test_name ${hit})
            add_test(${test_name} ${executable} --gtest_filter=${test_name} ${extra_args})
        endforeach()
        # Groups parametrized tests under a single ctest entry
        string(REGEX MATCHALL "INSTANTIATE_TEST_CASE_P\\(([^,]+), *([^,]+)" found_tests2 ${contents})
        foreach(hit ${found_tests2})
          string(SUBSTRING ${hit} 24 -1 test_name)
          string(REPLACE "," ";" test_name "${test_name}")
          list(GET test_name 0 filter_name)
          list(GET test_name 1 test_prefix)
          string(STRIP ${test_prefix} test_prefix)
          add_test(${test_prefix}.${filter_name} ${executable} --gtest_filter=${filter_name}* ${extra_args})
        endforeach()
    endforeach()
endfunction()
