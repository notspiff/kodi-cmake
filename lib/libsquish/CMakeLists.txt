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

if(SQUISH_NATIVE)
  core_add_library(squish-native 1)
else()
  if(NOT BOOTSTRAP_IN_TREE)
    core_add_library(squish)
  else()
    set(core_DEPENDS squish-native ${core_DEPENDS} CACHE STRING "" FORCE)
  endif()
endif()
