set(SOURCES libXBMC_codec.cpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CORE_SOURCE_DIR}/xbmc/addons/include)
add_options(CXX ALL_BUILDS "-fPIC")

core_add_library(XBMC_codec 1)
