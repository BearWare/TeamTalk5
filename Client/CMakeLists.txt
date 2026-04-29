project (TeamTalkClients)

option (BUILD_TEAMTALK_CLIENT_PRISM "Build TeamTalk clients with Prism screen reader support" ON)
if (BUILD_TEAMTALK_CLIENT_PRISM)
  add_subdirectory (Prism)
endif()

if (MSVC)

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

