if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(ARCH_DEFINES -D_LINUX -DTARGET_POSIX -DTARGET_LINUX)
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(ARCH x86_64-linux)
  endif()
  set(SYSTEM_DEFINES -D__STDC_CONSTANT_MACROS -D_FILE_DEFINED
                     -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)
  set(DEP_DEFINES -DLIRC_DEVICE=${LIRC_DEVICE})
endif()

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
