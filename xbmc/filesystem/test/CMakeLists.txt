set(SOURCES TestDirectory.cpp 
            TestFile.cpp
            TestFileFactory.cpp
            TestRarFile.cpp
            TestZipFile.cpp)

include_directories(${CORE_SOURCE_DIR}/lib/gtest/include)

core_add_test_library(filesystem_test)
