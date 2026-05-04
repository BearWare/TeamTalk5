option (BUILD_TEAMTALK_DOCUMENTATION_TEAMTALK "Build documentation for TeamTalk client and server applications" ON)

find_package (Doxygen)
if (DOXYGEN_FOUND)

  add_custom_command (OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/output/index.html
    COMMAND ${DOXYGEN_EXECUTABLE} TeamTalk5.dox
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating TeamTalk 5 client and server documentation" VERBATIM
    DEPENDS TeamTalk5.dox)

  add_custom_target (TeamTalk-dox ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/output/index.html)

  if (MSVC)
    find_program(HHC_EXE_PATH hhc.exe PATHS "C:/Program Files (x86)/HTML Help Workshop")
    if (HHC_EXE_PATH-NOTFOUND)
      message ("Cannot find HHC.exe for generating TeamTalk5.chm help file")
    else()
      file(TO_NATIVE_PATH ${HHC_EXE_PATH} HHC_EXE_PATH_NATIVE)
      # For some unknown reason hhc.exe returns 1 although no errors
      # or warnings are indicated. So this bat file is a workaround
      configure_file(${CMAKE_CURRENT_LIST_DIR}/hhc_exit0.bat_input ${CMAKE_CURRENT_BINARY_DIR}/hhc_exit0.bat @ONLY)

      add_custom_command (OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/output/TeamTalk5.chm
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/hhc_exit0.bat index.hhp
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output
        COMMENT "Generating TeamTalk 5 CHM file for documentation" VERBATIM
        DEPENDS TeamTalk-dox)
      add_custom_target (TeamTalk-dox-CHM ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/output/TeamTalk5.chm)
    endif()
  endif()

  install (DIRECTORY output DESTINATION Documentation/TeamTalk)
endif()
