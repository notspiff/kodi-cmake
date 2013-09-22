file(STRINGS ${XBMC_SOURCE_DIR}/xbmc/interfaces/json-rpc/schema/version.txt jsonrpc_version)

execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${XBMC_BINARY_DIR}/addons/xbmc.json/addon.xml)
configure_file(${XBMC_SOURCE_DIR}/addons/xbmc.json/addon.xml.in
               ${XBMC_BINARY_DIR}/addons/xbmc.json/addon.xml @ONLY)
