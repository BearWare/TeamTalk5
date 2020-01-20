cmake_minimum_required(VERSION 3.8)
project (TeamTalk5Test.NET)

enable_language (CSharp)

if (MSVC)
  option (BUILD_TEAMTALK_DOTNET_UNITTEST "Build TeamTalk 5 Unit Test for TeamTalk 5 Standard Edition" ON)
  option (BUILD_TEAMTALKPRO_DOTNET_UNITTEST "Build TeamTalk 5 Pro Unit Test for TeamTalk 5 Professional Edition" ON)

  if (BUILD_TEAMTALK_DOTNET_UNITTEST)
    add_library (TeamTalk5Test.NET SHARED TeamTalkTest.cs Properties/AssemblyInfo.cs)
    set_property (TARGET TeamTalk5Test.NET PROPERTY VS_DOTNET_REFERENCES
      "Microsoft.VisualStudio.QualityTools.UnitTestFramework"
      "System" "System.Core" "System.Drawing")
    target_link_libraries (TeamTalk5Test.NET TeamTalk5.NET)
  endif()

  if (BUILD_TEAMTALKPRO_DOTNET_UNITTEST)
    add_library (TeamTalk5ProTest.NET SHARED TeamTalkTest.cs Properties/AssemblyInfo.cs)
    target_compile_options (TeamTalk5ProTest.NET PUBLIC "/define:ENABLE_TEAMTALKPRO")
    set_property (TARGET TeamTalk5ProTest.NET PROPERTY VS_DOTNET_REFERENCES
      "Microsoft.VisualStudio.QualityTools.UnitTestFramework"
      "System" "System.Core" "System.Drawing")
    target_link_libraries (TeamTalk5ProTest.NET TeamTalk5Pro.NET)
  endif()
endif()
