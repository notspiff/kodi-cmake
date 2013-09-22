install(TARGETS xbmc-xrandr DESTINATION lib/xbmc)
install(FILES ${bindings} DESTINATION include/xbmc)
install(FILES ${cmake_files} ${CMAKE_BINARY_DIR}/xbmc-config.cmake
        DESTINATION lib/xbmc)
install(PROGRAMS ${CMAKE_BINARY_DIR}/xbmc ${CMAKE_BINARY_DIR}/xbmc-standalone
        DESTINATION bin)
install(FILES ${XBMC_SOURCE_DIR}/tools/Linux/FEH.py
        DESTINATION share/xbmc)
install_rename(${XBMC_SOURCE_DIR}/tools/Linux/xbmc-xsession.desktop
               ${CMAKE_INSTALL_PREFIX}/share/xsessions/XBMC.desktop)
                
install(FILES ${XBMC_SOURCE_DIR}/LICENSE.GPL
              ${XBMC_SOURCE_DIR}/docs/README.linux
        DESTINATION share/doc/xbmc)

install(FILES ${XBMC_SOURCE_DIR}/tools/Linux/xbmc.desktop
        DESTINATION share/applications)

install_rename(${XBMC_SOURCE_DIR}/tools/Linux/xbmc-48x48.png
               ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps/xbmc.png)
install_rename(${XBMC_SOURCE_DIR}/media/icon.png
               ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/256x256/apps/xbmc.png)

install(CODE "execute_process(COMMAND gtk-update-icon-cache -f -q -t ${DESTDIR}${CMAKE_INSTALL_PREFIX}/share/icons/hicolor ERROR_QUIET)")
