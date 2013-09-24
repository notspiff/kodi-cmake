set(SOURCES AddonPythonInvoker.cpp
            CallbackHandler.cpp
            LanguageHook.cpp
	    PythonInvoker.cpp
            XBPython.cpp
            swig.cpp
            PyContext.cpp)
            
xbmc_add_library(python_interface)
add_dependencies(python_interface libcpluff)
