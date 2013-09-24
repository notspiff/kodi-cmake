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

core_add_library(squish)
