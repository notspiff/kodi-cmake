include(${CORE_SOURCE_DIR}/project/cmake/scripts/common/macros.cmake)

core_find_versions()
configure_file(${CORE_SOURCE_DIR}/addons/xbmc.addon/addon.xml.in
               ${CMAKE_BINARY_DIR}/addons/xbmc.addon/addon.xml @ONLY)
configure_file(${CORE_SOURCE_DIR}/xbmc/CompileInfo.cpp.in
               ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/xbmc/CompileInfo.cpp @ONLY)
set(prefix ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR})
configure_file(${CORE_SOURCE_DIR}/project/cmake/templates/kodi-config.cmake.in
               ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/kodi/kodi-config.cmake @ONLY)
configure_file(${CORE_SOURCE_DIR}/project/cmake/templates/kodi-addon-helpers.cmake.in
               ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/kodi/kodi-addon-helpers.cmake @ONLY)

# Backwards compat with deprecation warning
configure_file(${CORE_SOURCE_DIR}/project/cmake/templates/xbmc-config.cmake.in
               ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/xbmc/xbmc-config.cmake @ONLY)
