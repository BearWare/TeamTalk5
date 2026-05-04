project (TeamTalk5ProServer.NET)

enable_language (CSharp)

if (MSVC)
  option (BUILD_TEAMTALK_SERVER_TEAMTALKSERVER_DOTNET "Build TeamTalk5ProServer.NET for TeamTalk 5 Professional Edition" ON)

  if (BUILD_TEAMTALK_SERVER_TEAMTALKSERVER_DOTNET)
    add_executable (TeamTalk5ProServer.NET Properties/AssemblyInfo.cs
      Program.cs ServerCallback.cs ServerLogger.cs)
    set_property(TARGET TeamTalk5ProServer.NET PROPERTY VS_DOTNET_REFERENCES "System")
    set_property(TARGET TeamTalk5ProServer.NET PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Library/TeamTalk_DLL")
    target_link_libraries (TeamTalk5ProServer.NET TeamTalk5Pro.NET)

    install (TARGETS TeamTalk5ProServer.NET DESTINATION Server/TeamTalkServer.NET)
  endif()
endif()
