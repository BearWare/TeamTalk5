cmake_minimum_required(VERSION 3.8)
project (TeamTalk5ProServer.NET)

enable_language (CSharp)

if (MSVC)
  option (TEAMTALKPRO "Build TeamTalk5ProServer.NET for TeamTalk 5 Professional Edition" ON)

  if (TEAMTALKPRO)
    add_executable (TeamTalk5ProServer.NET Properties/AssemblyInfo.cs
      Program.cs ServerCallback.cs ServerLogger.cs)
    set_property(TARGET TeamTalk5ProServer.NET PROPERTY VS_DOTNET_REFERENCES "System")
    target_link_libraries (TeamTalk5ProServer.NET TeamTalk5Pro.NET)
  endif()
endif()
