project (jTeamTalkSpamBot)

find_package (Java)

if (Java_FOUND)

  include (UseJava)

  add_subdirectory(deps)

  option (BUILD_TEAMTALK_CLIENT_JSPAMBOT "Build TeamTalk Anti Spam Bot" ON)

  set(CMAKE_JAVA_COMPILE_FLAGS "-source" "11" "-target" "11")

  set (JAVA_SOURCES src/SpamBotSession.java src/WebLogin.java
    src/Main.java src/TeamTalkServer.java src/IPBan.java
    src/BadWords.java src/Abuse.java src/AbuseDB.java)

  if (BUILD_TEAMTALK_CLIENT_JSPAMBOT)

    if (BUILD_TEAMTALK_LIBRARY_JAR)
      set (TEAMTALKJAR_LIBRARY TeamTalk5JAR)
    else()
      find_jar (TEAMTALKJAR_LIBRARY NAMES TeamTalk5
        PATHS ${CMAKE_CURRENT_LIST_DIR}/../../Library/TeamTalkJNI/libs)
    endif()

    if (NOT TEAMTALKJAR_LIBRARY)
      message("Cannot find TeamTalk5.jar")
    else()
      add_jar (jTeamTalkSpamBot INCLUDE_JARS ${TEAMTALKJAR_LIBRARY}
        ${CMAKE_CURRENT_BINARY_DIR}/deps/org-json.jar
        ENTRY_POINT Main
        SOURCES ${JAVA_SOURCES})
      install_jar (jTeamTalkSpamBot DESTINATION Client/jSpamBot)
    endif()
  endif()
endif()
