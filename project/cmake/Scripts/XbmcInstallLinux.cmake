set(libdir ${CMAKE_INSTALL_PREFIX}/lib)
set(bindir ${CMAKE_INSTALL_PREFIX}/bin)
configure_file(${XBMC_SOURCE_DIR}/tools/Linux/xbmc.sh.in
               build/scripts/xbmc @ONLY)
configure_file(${XBMC_SOURCE_DIR}/tools/Linux/xbmc-standalone.sh.in
               build/scripts/xbmc-standalone @ONLY)

install(TARGETS xbmc-xrandr DESTINATION lib/xbmc)
install(FILES ${bindings} DESTINATION include/xbmc)
install(FILES ${cmake_files} ${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR}/xbmc-config.cmake
        DESTINATION lib/xbmc)
install(PROGRAMS ${CMAKE_BINARY_DIR}/build/scripts/xbmc
                 ${CMAKE_BINARY_DIR}/build/scripts/xbmc-standalone
        DESTINATION bin)
install(FILES ${XBMC_SOURCE_DIR}/tools/Linux/FEH.py
        DESTINATION share/xbmc)

install(FILES ${XBMC_SOURCE_DIR}/tools/Linux/xbmc-xsession.desktop
        RENAME XBMC.desktop
        DESTINATION share/xsessions)
                
install(FILES ${XBMC_SOURCE_DIR}/LICENSE.GPL
              ${XBMC_SOURCE_DIR}/docs/README.linux
        DESTINATION share/doc/xbmc)

install(FILES ${XBMC_SOURCE_DIR}/tools/Linux/xbmc.desktop
        DESTINATION share/applications)

foreach(texture ${XBT_FILES})
  get_filename_component(dir ${texture} PATH)
  install(FILES ${CMAKE_BINARY_DIR}/${texture}
          DESTINATION share/xbmc/${dir})
endforeach()

foreach(wraplib ${WRAP_FILES})
  get_filename_component(dir ${wraplib} PATH)
  install(PROGRAMS ${CMAKE_BINARY_DIR}/${wraplib}
          DESTINATION lib/xbmc/${dir})
endforeach()

foreach(file ${install_data})
  get_filename_component(dir ${file} PATH)
  install(FILES ${CMAKE_BINARY_DIR}/${file}
          DESTINATION share/xbmc/${dir})
endforeach()

if(ENABLE_SKIN_TOUCHED)
  install(DIRECTORY ${CMAKE_BINARY_DIR}/addons/skin.touched
          DESTINATION share/xbmc/addons
          FILES_MATCHING
          PATTERN "*.jpg;*.png" EXCLUDE)
endif()
install(FILES ${XBMC_SOURCE_DIR}/tools/Linux/xbmc-48x48.png
        RENAME xbmc.png
        DESTINATION share/icons/hicolor/48x48/apps)
install(FILES ${XBMC_SOURCE_DIR}/media/icon.png
        RENAME xbmc.png
        DESTINATION share/icons/hicolor/256x256/apps)

install(CODE "execute_process(COMMAND gtk-update-icon-cache -f -q -t $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/icons/hicolor ERROR_QUIET)")
