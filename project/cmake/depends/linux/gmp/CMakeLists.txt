project(gmp)

cmake_minimum_required(VERSION 2.8)

if(CMAKE_SYSTEM_NAME MATCHES Linux)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "i.86")
    set(ABI ABI=32)
  endif()
endif()

include(ExternalProject)
externalproject_add(gmp
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}
                    UPDATE_COMMAND ""
                    CONFIGURE_COMMAND <SOURCE_DIR>/configure 
                        --prefix=${OUTPUT_DIR}
                        --disable-shared
                        ${ABI}
                    INSTALL_COMMAND ""
                    BUILD_IN_SOURCE 1)
                  
install(CODE "execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
