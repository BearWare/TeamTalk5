option (BUILD_TEAMTALK_CLIENT_DOCS "Build documentation for TeamTalk client and server applications" ON)

find_package (Doxygen)
if (DOXYGEN_FOUND)

  add_custom_command (OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/output/index.html
    COMMAND ${DOXYGEN_EXECUTABLE} TeamTalk5.dox
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating TeamTalk 5 client and server documentation" VERBATIM
    DEPENDS TeamTalk5.dox)
    
  add_custom_target (TeamTalk-dox ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/output/index.html)

  install (DIRECTORY output DESTINATION Documentation/TeamTalk)
endif()
