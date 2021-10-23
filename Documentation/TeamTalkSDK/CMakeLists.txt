option (BUILD_TEAMTALK_SDK_DOCS "Build documentation for TeamTalk SDK" ON)

find_package (Doxygen)
if (DOXYGEN_FOUND)

  add_custom_command (OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/C-API/index.html
    COMMAND ${DOXYGEN_EXECUTABLE} TeamTalk.dox
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating TeamTalk 5 SDK C-API documentation"
    DEPENDS TeamTalk.dox)
    
  add_custom_target (TeamTalkSDK-dox ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/C-API/index.html)

  install (DIRECTORY C-API DESTINATION Documentation/TeamTalkSDK)
  
  add_custom_command (OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Java/index.html
    COMMAND ${DOXYGEN_EXECUTABLE} TeamTalk.Java.dox
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating TeamTalk 5 SDK Java API documentation"
    DEPENDS TeamTalk.Java.dox)
    
  add_custom_target (TeamTalkSDK-Java-dox ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Java/index.html)

  install (DIRECTORY Java DESTINATION Documentation/TeamTalkSDK)
  
  add_custom_command (OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/NET/index.html
    COMMAND ${DOXYGEN_EXECUTABLE} TeamTalk.NET.dox
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating TeamTalk 5 SDK .NET documentation"
    DEPENDS TeamTalk.NET.dox)
    
  add_custom_target (TeamTalkSDK-NET-dox ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/NET/index.html)

  install (DIRECTORY NET DESTINATION Documentation/TeamTalkSDK)
  
endif()
