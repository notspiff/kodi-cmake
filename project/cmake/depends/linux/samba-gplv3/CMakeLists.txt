project(samba-gplv3)

cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
externalproject_add(samba-gplv3
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}/source3
                    UPDATE_COMMAND ""
                    CONFIGURE_COMMAND <SOURCE_DIR>/configure 
                      --prefix=${OUTPUT_DIR}
                      --without-cluster-support
                      --disable-swat
                      --without-ldap
                      --without-pam
                      --without-pam_smbpass
                      --without-fhs
                      --with-libtalloc=no
                      --with-libtdb=no
                      --without-winbind
                      --disable-cups
                      --without-ads
                      --disable-avahi
                      --disable-fam
                      --without-libaddns
                      --without-libnetapi
                      --without-dnsupdate
                      --without-libsmbsharemodes
                    BUILD_COMMAND make libsmbclient
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)
                  
install(CODE "execute_process(COMMAND make -C source3 installlibsmbclient WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
