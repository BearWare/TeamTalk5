project (TeamTalk5.NET LANGUAGES CSharp)

if (MSVC)
  option (BUILD_TEAMTALK_LIBRARY_DLL_DOTNET "Build TeamTalk 5 .NET DLL for TeamTalk 5 Standard Edition" ON)
  option (BUILD_TEAMTALK_LIBRARY_DLLPRO_DOTNET "Build TeamTalk 5 Pro .NET DLL for TeamTalk 5 Professional Edition" ON)

  if (BUILD_TEAMTALK_LIBRARY_DLL_DOTNET)
    add_library (TeamTalk5.NET SHARED
      AssemblyInfo.cs TeamTalk.cs TeamTalkInterop.cs)
    set_property (TARGET TeamTalk5.NET PROPERTY VS_DOTNET_REFERENCES "System" "System.Drawing" "System.Windows.Forms")
    if (BUILD_TEAMTALK_LIBRARY_DLL)
      target_link_libraries (TeamTalk5.NET PRIVATE TeamTalk5DLL)
    endif()
    install (TARGETS TeamTalk5.NET DESTINATION Library/TeamTalk.NET)
  endif()

  if (BUILD_TEAMTALK_LIBRARY_DLLPRO_DOTNET)
    add_library (TeamTalk5Pro.NET SHARED
      AssemblyInfo.cs TeamTalk.cs TeamTalkInterop.cs
      TeamTalkSrv.cs TeamTalkSrvBase.cs TeamTalkSrvInterop.cs)
    target_compile_options (TeamTalk5Pro.NET PUBLIC "/define:ENABLE_TEAMTALKPRO")
    set_property (TARGET TeamTalk5Pro.NET PROPERTY VS_DOTNET_REFERENCES "System" "System.Drawing" "System.Windows.Forms")
    if (BUILD_TEAMTALK_LIBRARY_DLLPRO)
      target_link_libraries (TeamTalk5Pro.NET PRIVATE TeamTalk5ProDLL)
    endif()
    install (TARGETS TeamTalk5Pro.NET DESTINATION Library/TeamTalk.NET)
  endif()
endif()

add_subdirectory (TeamTalkTest.NET)
