configure_file(${CORE_SOURCE_DIR}/tools/Linux/xbmc.sh.in
               ${CORE_BUILD_DIR}/scripts/xbmc @ONLY)
configure_file(${CORE_SOURCE_DIR}/tools/Linux/xbmc-standalone.sh.in
               ${CORE_BUILD_DIR}/scripts/xbmc-standalone @ONLY)

install(TARGETS xbmc-xrandr DESTINATION ${libdir}/xbmc)
install(FILES ${addon_bindings} DESTINATION ${includedir}/xbmc)
install(FILES ${cmake_files}
              ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/modules/xbmc-config.cmake
              ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/modules/xbmc-addon-helpers.cmake
        DESTINATION ${libdir}/xbmc)
install(PROGRAMS ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/scripts/xbmc
                 ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/scripts/xbmc-standalone
        DESTINATION ${bindir})
install(FILES ${CORE_SOURCE_DIR}/tools/Linux/FEH.py
        DESTINATION ${datarootdir}/xbmc)

install(FILES ${CORE_SOURCE_DIR}/tools/Linux/xbmc-xsession.desktop
        RENAME XBMC.desktop
        DESTINATION ${datarootdir}/xsessions)
                
install(FILES ${CORE_SOURCE_DIR}/LICENSE.GPL
              ${CORE_SOURCE_DIR}/docs/README.linux
        DESTINATION ${datarootdir}/doc/xbmc)

install(FILES ${CORE_SOURCE_DIR}/tools/Linux/xbmc.desktop
        DESTINATION ${datarootdir}/applications)

foreach(texture ${XBT_FILES})
  string(REPLACE "${CMAKE_BINARY_DIR}/" "" dir ${texture})
  get_filename_component(dir ${dir} PATH)
  install(FILES ${texture}
          DESTINATION ${datarootdir}/xbmc/${dir})
endforeach()

foreach(wraplib ${WRAP_FILES})
  get_filename_component(dir ${wraplib} PATH)
  install(PROGRAMS ${CMAKE_BINARY_DIR}/${wraplib}
          DESTINATION ${libdir}/xbmc/${dir})
endforeach()

foreach(file ${install_data})
  get_filename_component(dir ${file} PATH)
  install(FILES ${CMAKE_BINARY_DIR}/${file}
          DESTINATION ${datarootdir}/xbmc/${dir})
endforeach()

install(CODE "file(STRINGS ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/extra-installs dirs)
              foreach(dir \${dirs})
                file(GLOB_RECURSE FILES RELATIVE ${CMAKE_BINARY_DIR} \${dir}/*)
                foreach(file \${FILES})
                  get_filename_component(dir \${file} PATH)
                  file(INSTALL \${file} DESTINATION ${datarootdir}/xbmc/\${dir})
                endforeach()
              endforeach()")
if(NOT "$ENV{DESTDIR}" STREQUAL "")
  set(DESTDIR ${CMAKE_BINARY_DIR}/$ENV{DESTDIR})
endif()
foreach(subdir ${build_dirs})
  if(NOT subdir MATCHES xbmc-platform)
    string(REPLACE " " ";" subdir ${subdir})
    list(GET subdir 0 id)
    install(CODE "execute_process(COMMAND make -C ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/${id}/src/${id}-build install DESTDIR=${DESTDIR})")
  endif()
endforeach()

install(FILES ${CORE_SOURCE_DIR}/media/icon48x48.png
        RENAME xbmc.png
        DESTINATION ${datarootdir}/icons/hicolor/48x48/apps)
install(FILES ${CORE_SOURCE_DIR}/media/icon256x256.png
        RENAME xbmc.png
        DESTINATION ${datarootdir}/icons/hicolor/256x256/apps)

install(CODE "execute_process(COMMAND gtk-update-icon-cache -f -q -t $ENV{DESTDIR}${datarootdir}/icons/hicolor ERROR_QUIET)")
