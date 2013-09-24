message(STATUS "system type: ${CMAKE_SYSTEM_NAME}")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(LIRC_DEVICE          "\"/dev/lircd\"" CACHE STRING "LIRC device to use")
  set(ARCH_DEFINES -D_LINUX -DTARGET_POSIX -DTARGET_LINUX)
  set(SYSTEM_DEFINES -D__STDC_CONSTANT_MACROS -D_FILE_DEFINED
                     -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)
  set(DEP_DEFINES -DLIRC_DEVICE=${LIRC_DEVICE})

  if(WITH_CPU)
    set(CPU ${WITH_CPU})
  else()
    set(CPU ${CMAKE_SYSTEM_PROCESSOR})
  endif()

  if(WITH_ARCH)
    set(ARCH ${WITH_ARCH})
  else()
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
      set(ARCH x86_64-linux)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i.86")
      set(ARCH i486-linux)
    else()
      message(WARNING "unknown CPU: ${CPU}")
    endif()
  endif()
else()
  message(WARNING "unsupported system")
endif()
message(STATUS "CPU: ${CPU}, ARCH: ${ARCH}")

include(CheckCXXSourceCompiles)

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

check_type(string std::u16string HAVE_STD__U16_STRING)
check_type(string std::u32string HAVE_STD__U32_STRING)
check_type(string char16_t HAVE_CHAR16_T)
check_type(string char32_t HAVE_CHAR32_T)
check_type(stdint.h uint_least16_t HAVE_STDINT_H)
