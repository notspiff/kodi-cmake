include(CMakeParseArguments)
include(${CORE_SOURCE_DIR}/project/cmake/scripts/common/macros.cmake)
core_file_read_filtered(version_list ${CORE_SOURCE_DIR}/version.txt)
string(REPLACE " " ";" version_list "${version_list}")
cmake_parse_arguments(APP "" "VERSION_MAJOR;VERSION_MINOR;VERSION_TAG;VERSION_CODE;ADDON_API" "" ${version_list})
configure_file(${CORE_SOURCE_DIR}/addons/xbmc.addon/addon.xml.in
               ${CMAKE_BINARY_DIR}/addons/xbmc.addon/addon.xml @ONLY)
configure_file(${CORE_SOURCE_DIR}/xbmc/CompileInfo.cpp.in
               ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/xbmc/CompileInfo.cpp @ONLY)
configure_file(${CORE_SOURCE_DIR}/project/cmake/templates/xbmc-config.cmake.in
               ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/xbmc/xbmc-config.cmake @ONLY)
configure_file(${CORE_SOURCE_DIR}/project/cmake/templates/xbmc-addon-helpers.cmake.in
               ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/xbmc/xbmc-addon-helpers.cmake @ONLY)
