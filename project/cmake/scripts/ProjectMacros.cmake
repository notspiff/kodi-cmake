function(pack_xbt input output relative)
  file(GLOB_RECURSE MEDIA_FILES ${input}/*)
  get_filename_component(dir ${output} PATH)
  add_custom_command(OUTPUT  ${output}
                     COMMAND ${CMAKE_COMMAND} -E make_directory ${dir}
                     COMMAND ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/texturepacker/TexturePacker
                     ARGS    -input ${input}
                             -output ${output}
                             -dupecheck
                     DEPENDS ${MEDIA_FILES} TexturePacker)
  list(APPEND XBT_FILES ${output})
  set(XBT_FILES ${XBT_FILES} PARENT_SCOPE)
endfunction()

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

function(prepare_addon_env)
  set(prefix ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR})
  file(COPY ${addon_bindings} DESTINATION ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/include/xbmc)
  configure_file(${CORE_SOURCE_DIR}/project/cmake/templates/xbmc-config.cmake.in
                 ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/xbmc/xbmc-config.cmake @ONLY)
  file(COPY ${cmake-files} DESTINATION ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/xbmc)
endfunction()
