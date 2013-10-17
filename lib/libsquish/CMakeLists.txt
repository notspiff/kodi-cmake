set(SOURCES alpha.cpp
            clusterfit.cpp
            colourblock.cpp
            colourfit.cpp
            colourset.cpp
            maths.cpp
            rangefit.cpp
            singlecolourfit.cpp
            squish.cpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

find_package(SSE)
if(SSE2_FOUND)
  list(APPEND CMAKE_CXX_FLAGS -DSQUISH_USE_SSE=2)
elseif(SSE_FOUND)
  list(APPEND CMAKE_CXX_FLAGS -DSQUISH_USE_SSE=1)
endif()

core_add_library(squish)
