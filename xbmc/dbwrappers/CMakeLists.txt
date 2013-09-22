set(SOURCES Database.cpp
            dataset.cpp
            qry_dat.cpp
            sqlitedataset.cpp)

if(MYSQLCLIENT_FOUND)
  list(APPEND SOURCES mysqldataset.cpp)
endif()
  
xbmc_add_library(dbwrappers)
