project (jTeamTalk)

find_package (Java)

if (Java_FOUND)

  include (UseJava)

  option (BUILD_TEAMTALK_CLIENT_JTEAMTALK "Build TeamTalk Client for Java" ON)

  set (JAVA_SOURCES src/TeamTalkClient.java)

  if (BUILD_TEAMTALK_CLIENT_JTEAMTALK)

    if (BUILD_TEAMTALK_JAR)
      set (TEAMTALKJAR_LIBRARY TeamTalk5JAR)
    else()
      find_jar (TEAMTALKJAR_LIBRARY NAMES TeamTalk5
        PATHS ${CMAKE_CURRENT_LIST_DIR}/../../Library/TeamTalkJNI/libs)
    endif()

    if (NOT TEAMTALKJAR_LIBRARY)
      message("Cannot find TeamTalk5.jar")
    else()
      add_jar (jTeamTalk INCLUDE_JARS ${TEAMTALKJAR_LIBRARY}
        ENTRY_POINT TeamTalkClient
        SOURCES ${JAVA_SOURCES})
    endif()
  endif()

endif()
