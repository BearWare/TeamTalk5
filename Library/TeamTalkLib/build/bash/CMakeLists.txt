project(Bash)

##################################################
# Bash (MSYS2/Cygwin)
##################################################

if (MSVC AND TOOLCHAIN_BUILD_EXTERNALPROJECTS)
  find_program(BASH_EXE_PATH bash.exe PATHS C:/msys64/usr/bin C:/cygwin64/bin $ENV{PATH} NO_DEFAULT_PATH)

  if (BASH_EXE_PATH-NOTFOUND)
    message(WARNING "MSYS2 or Cygwin bash.exe is required")
  else()
    message(STATUS "Found bash.exe (MSYS2/Cygwin): ${BASH_EXE_PATH}")
  endif()

  file(TO_NATIVE_PATH ${BASH_EXE_PATH} BASH_EXE_PATH_NATIVE)

  set(BASH_EXE_PATH ${BASH_EXE_PATH} PARENT_SCOPE)
  set(BASH_EXE_PATH_NATIVE ${BASH_EXE_PATH_NATIVE} PARENT_SCOPE)
endif()
