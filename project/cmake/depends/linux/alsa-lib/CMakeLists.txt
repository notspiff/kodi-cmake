project(alsa-lib)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
externalproject_add(alsa-lib
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND autoreconf -vif WORKING_DIRECTORY <SOURCE_DIR>
                    CONFIGURE_COMMAND <SOURCE_DIR>/configure 
                      --prefix=${OUTPUT_DIR}
                      --with-alsa-devdir=/dev/snd/
                      --with-plugindir=/system/usr/lib/alsa-lib
                      --with-configdir=/system/usr/share/alsa
                      --with-ctl-plugins=ext
                      --with-pcm-plugins="copy,linear,route,mulaw,alaw,adpcm,rate,plug,multi,file,null,empty,share,meter,hooks,lfloat,ladspa,asym,iec958,softvol,extplug,ioplug,mmap_emul"
                      --disable-resmgr --enable-aload --enable-mixer
                      --enable-pcm  --disable-rawmidi --enable-hwdep
                      --disable-seq  --disable-alisp  --disable-old-symbols
                      --disable-python --with-softfloat=yes --with-libdl=yes
                      --with-pthread=yes --with-librt=no --disable-shared
                    BUILD_COMMAND make -C src
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)
                  
install(CODE "execute_process(COMMAND make -C src install-libLTLIBRARIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND make -C include install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND make -C utils install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
