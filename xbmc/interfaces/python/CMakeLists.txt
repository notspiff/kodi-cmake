macro(generate_file file)
  add_custom_command(OUTPUT ${file}.cpp
                     COMMAND ${SWIG_EXECUTABLE}
                     ARGS -w401 -c++ -o ${file}.xml -xml -I${XBMC_SOURCE_DIR}/xbmc -xmllang python ${CMAKE_CURRENT_SOURCE_DIR}/../swig/${file}
                     COMMAND ${Java_JAVA_EXECUTABLE}
                     ARGS -cp ${GROOVY_DIR}/groovy-all-1.8.9.jar:${GROOVY_DIR}/commons-lang-2.6.jar:${XBMC_SOURCE_DIR}/tools/codegenerator:${CMAKE_CURRENT_SOURCE_DIR} groovy.ui.GroovyMain ${XBMC_SOURCE_DIR}/tools/codegenerator/Generator.groovy ${file}.xml ${CMAKE_CURRENT_SOURCE_DIR}/PythonSwig.cpp.template ${file}.cpp ${CMAKE_CURRENT_SOURCE_DIR}/generated/doxygenxml
                     DEPENDS doxygen 
                     ${CMAKE_CURRENT_SOURCE_DIR}/../swig/${file} ${INTERFACE_FILES}
                     IMPLICIT_DEPENDS CXX ${INTERFACE_FILES})
  list(APPEND SOURCES ${file}.cpp)
endmacro()

set(SOURCES AddonPythonInvoker.cpp
            CallbackHandler.cpp
            LanguageHook.cpp
	    PythonInvoker.cpp
            XBPython.cpp
            swig.cpp
            PyContext.cpp)
  
xbmc_add_library(python_interface)
add_dependencies(python_interface libcpluff)

# The generated bindings
set(INPUTS AddonModuleXbmcaddon.i
           AddonModuleXbmcgui.i
           AddonModuleXbmc.i
           AddonModuleXbmcplugin.i
           AddonModuleXbmcvfs.i)

set(SOURCES "")

set(GROOVY_DIR ${XBMC_SOURCE_DIR}/lib/groovy)

file(GLOB INTERFACE_FILES ${XBMC_SOURCE_DIR}/xbmc/lib/legacy)

find_package(Java REQUIRED)
find_package(SWIG REQUIRED)

foreach(INPUT ${INPUTS})
  generate_file(${INPUT})
endforeach()

add_custom_target(doxygen DEPENDS ${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR}/doxygen-gen)
find_package(Doxygen)
if(DOXYGEN_FOUND)
  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR}/doxygen-gen
                     COMMAND ${DOXYGEN_EXECUTABLE} ARGS ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
                     COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR}/doxygen-gen
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                     MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
else(DOXYGEN_FOUND)
  message(STATUS "Warning: No doxygen installed. The Api will not have any docstrings.")
  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/${XBMC_BUILD_DIR}/doxygen-gen
                   COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/generated/doxygenxml)
endif(DOXYGEN_FOUND)
                   
xbmc_add_library(python_binding)
foreach(INPUT ${INPUTS})
  list(APPEND INPUT2 gen-${INPUT}.cpp)
endforeach()
add_dependencies(python_binding ${INPUT2} libcpluff)
