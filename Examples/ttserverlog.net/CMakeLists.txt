cmake_minimum_required(VERSION 3.8)
project (ttserverlog.NET)

enable_language (CSharp)

if (MSVC)
  option (BUILD_TEAMTALKCLIENT_LOG_DOTNET_EXAMPLE "Build ttserverlog.NET for TeamTalk 5 Standard Edition" ON)
  option (BUILD_TEAMTALKCLIENTPRO_LOG_DOTNET_EXAMPLE "Build ttproserverlog.NET for TeamTalk 5 Professional Edition" ON)

  if (BUILD_TEAMTALKCLIENT_LOG_DOTNET_EXAMPLE)
    add_executable (ttserverlog.NET Program.cs)
    set_property(TARGET ttserverlog.NET PROPERTY VS_DOTNET_REFERENCES "System")
    target_link_libraries (ttserverlog.NET TeamTalk5.NET)
  endif()

  if (BUILD_TEAMTALKCLIENTPRO_LOG_DOTNET_EXAMPLE)
    add_executable (ttproserverlog.NET Program.cs)
    set_property(TARGET ttproserverlog.NET PROPERTY VS_DOTNET_REFERENCES "System")
    target_compile_options (ttproserverlog.NET PUBLIC "/define:ENABLE_ENCRYPTION")
    target_link_libraries (ttproserverlog.NET TeamTalk5Pro.NET)
  endif()
endif()
