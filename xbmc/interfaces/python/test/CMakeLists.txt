set(SOURCES TestSwig.cpp)

include_directories(${CORE_SOURCE_DIR}/lib/gtest/include)

core_add_test_library(python_test)
