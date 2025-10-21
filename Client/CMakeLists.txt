project (TeamTalkClients)

if (MSVC)

  option (BUILD_TEAMTALK_CLIENT_TOLK "Build TeamTalk clients with Tolk" ON)
  if (BUILD_TEAMTALK_CLIENT_TOLK)
    add_subdirectory (Tolk)
  endif()

  add_subdirectory (TeamTalkClassic)
  if (CMAKE_CSharp_COMPILER)
    add_subdirectory (TeamTalkApp.NET)
    add_subdirectory (ttserverlog.NET)
  else()
    message(WARNING "No C# compiler detected. Skipping .NET client examples")
  endif()
endif()

add_subdirectory (jSpamBot)
add_subdirectory (jTeamTalk)
add_subdirectory (qtTeamTalk)
add_subdirectory (ttserverlog)

