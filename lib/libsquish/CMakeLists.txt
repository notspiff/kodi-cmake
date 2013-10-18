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
  add_definitions(-DSQUISH_USE_SSE=2)
elseif(SSE_FOUND)
  add_definitions(-DSQUISH_USE_SSE=1)
endif()

# Always compile release - it's insanely slow in debug
list(APPEND CMAKE_CXX_FLAGS -O2)
add_definitions(-DNDEBUG)

core_add_library(squish)
