project(NASM)

include(ExternalProject)

##################################################
# NASM
##################################################

if (MSVC)
  set(NASM_INSTALL_DIR ${CMAKE_BINARY_DIR}/nasm/nasm-3.01)

  find_program(NASM_EXECUTABLE nasm.exe PATHS "C:/Program Files/NASM")

  if(NOT NASM_EXECUTABLE)
    ExternalProject_Add(nasm-bin
      URL               https://www.nasm.us/pub/nasm/releasebuilds/3.01/win64/nasm-3.01-win64.zip
      URL_HASH          SHA256=e0ba5157007abc7b1a65118a96657a961ddf55f7e3f632ee035366dfce039ca4
      DOWNLOAD_DIR      ${CMAKE_BINARY_DIR}/nasm
      SOURCE_DIR        ${NASM_INSTALL_DIR}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND     ""
      INSTALL_COMMAND   ""
    )
    set(NASM_EXECUTABLE ${NASM_INSTALL_DIR}/nasm.exe)
    set(NASM_EXE_DIRECTORY ${NASM_INSTALL_DIR})
  else()
    get_filename_component(NASM_EXE_DIRECTORY ${NASM_EXECUTABLE} DIRECTORY)
  endif()

  file(TO_NATIVE_PATH ${NASM_EXE_DIRECTORY} NASM_EXE_DIRECTORY_NATIVE)
  message(STATUS "Found NASM: ${NASM_EXECUTABLE}")

  set(NASM_EXECUTABLE ${NASM_EXECUTABLE} PARENT_SCOPE)
  set(NASM_EXE_DIRECTORY ${NASM_EXE_DIRECTORY} PARENT_SCOPE)
  set(NASM_EXE_DIRECTORY_NATIVE ${NASM_EXE_DIRECTORY_NATIVE} PARENT_SCOPE)
  if(TARGET nasm-bin)
    set(NASM_TARGET nasm-bin PARENT_SCOPE)
  endif()
endif()
