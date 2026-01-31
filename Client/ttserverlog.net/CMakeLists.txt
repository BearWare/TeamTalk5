project (ttserverlog.NET)

enable_language (CSharp)

if (MSVC)
  option (BUILD_TEAMTALK_CLIENT_TTSERVERLOG_DOTNET "Build ttserverlog.NET C# client example" ON)

  if (BUILD_TEAMTALK_CLIENT_TTSERVERLOG_DOTNET)
    add_executable (ttserverlog.NET Program.cs)
    set_property(TARGET ttserverlog.NET PROPERTY VS_DOTNET_REFERENCES "System")
    set_property(TARGET ttserverlog.NET PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Library/TeamTalk_DLL")
    target_link_libraries (ttserverlog.NET TeamTalk5.NET)
    install (TARGETS ttserverlog.NET DESTINATION Client/ttserverlog.net)
  endif()
endif()
