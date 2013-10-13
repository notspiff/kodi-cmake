# This script configures the build for a given architecture.
# Flags and stringified arch is set up.
# General compiler tests belongs here.
#
# On return, the following variables are set:
# CMAKE_SYSTEM_NAME - a lowercased system name
# CPU - the CPU on the target
# ARCH - the system architecture
# ARCH_DEFINES - list of compiler definitions for this architecture
# SYSTEM_DEFINES - list of compiler definitions for this system
# DEP_DEFINES - compiler definitions for system dependencies (e.g. LIRC)
# + the results of compiler tests etc.

include(CheckCXXSourceCompiles)

# Macro to check if a given type exists in a given header
# Arguments:
#   header the header to check
#   type   the type to check for existence
#   var    the compiler definition to set if type exists
# On return:
#   If type was found, the definition is added to SYSTEM_DEFINES
macro(check_type header type var)
  check_cxx_source_compiles("#include <${header}>
                             int main()
                             {
                               ${type} s;
                             }" ${var})
  if(${var})
    list(APPEND SYSTEM_DEFINES -D${var}=1)
  endif()
endmacro()

# -------- Main script --------- 
message(STATUS "system type: ${CMAKE_SYSTEM_NAME}")
string(TOLOWER ${CMAKE_SYSTEM_NAME} CORE_SYSTEM_NAME)

if(WITH_CPU)
  set(CPU ${WITH_CPU})
else()
  set(CPU ${CMAKE_SYSTEM_PROCESSOR})
endif()

# system specific arch setup
include(${PROJECT_SOURCE_DIR}/scripts/${CORE_SYSTEM_NAME}/archsetup.cmake)

message(STATUS "CPU: ${CPU}, ARCH: ${ARCH}")

check_type(string std::u16string HAVE_STD__U16_STRING)
check_type(string std::u32string HAVE_STD__U32_STRING)
check_type(string char16_t HAVE_CHAR16_T)
check_type(string char32_t HAVE_CHAR32_T)
check_type(stdint.h uint_least16_t HAVE_STDINT_H)
