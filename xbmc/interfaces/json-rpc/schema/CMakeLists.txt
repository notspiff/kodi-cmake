add_executable(JsonSchemaBuilder ${CORE_SOURCE_DIR}/tools/depends/native/JsonSchemaBuilder/src/JsonSchemaBuilder.cpp)

# Needed due to link wrapper
add_dependencies(JsonSchemaBuilder ffmpeg)

set(JSON_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/version.txt
              ${CMAKE_CURRENT_SOURCE_DIR}/license.txt
              ${CMAKE_CURRENT_SOURCE_DIR}/methods.json
              ${CMAKE_CURRENT_SOURCE_DIR}/types.json
              ${CMAKE_CURRENT_SOURCE_DIR}/notifications.json)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ServiceDescription.h
                   COMMAND ${CMAKE_CURRENT_BINARY_DIR}/JsonSchemaBuilder ${JSON_SRCS}
                   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
                   DEPENDS ${JSON_SRCS}
                   COMMENT "Generating ServiceDescription.h")

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/addons/xbmc.json/addon.xml
                   COMMAND ${CMAKE_COMMAND}
                           -DXBMC_SOURCE_DIR=${CORE_SOURCE_DIR}
                           -DXBMC_BINARY_DIR=${CMAKE_BINARY_DIR}
                           -P ${CMAKE_CURRENT_SOURCE_DIR}/GenerateAddonXml.cmake
                   WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                   DEPENDS ${JSON_SRCS} ${CORE_SOURCE_DIR}/addons/xbmc.json/addon.xml.in
                   COMMENT "Generating xbmc.json/addon.xml")

add_custom_target(generate_json_header ALL
                  DEPENDS JsonSchemaBuilder
                           ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ServiceDescription.h
                           ${CMAKE_BINARY_DIR}/addons/xbmc.json/addon.xml)
list(APPEND install_data addons/xbmc.json/addon.xml)

set(install_data ${install_data} PARENT_SCOPE)
