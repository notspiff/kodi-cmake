set(SOURCES alpha.cpp
            clusterfit.cpp
            colourblock.cpp
            colourfit.cpp
            colourset.cpp
            maths.cpp
            rangefit.cpp
            singlecolourfit.cpp
            squish.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(SSE)
if(SSE2_FOUND)
  add_definitions(-DSQUISH_USE_SSE=2)
elseif(SSE_FOUND)
  add_definitions(-DSQUISH_USE_SSE=1)
endif()

# Always compile release - it's insanely slow in debug
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
add_definitions(-DNDEBUG)

if(SQUISH_TARGET)
  core_add_library(${SQUISH_TARGET} 1)
else()
  core_add_library(squish)
endif()
