cmake_minimum_required(VERSION 3.10)
project (TeamTalk5Test.NET)

enable_language (CSharp)

if (MSVC)
  option (BUILD_TEAMTALK_LIBRARY_UNITTEST_DOTNET "Build TeamTalk 5 Unit Test for TeamTalk 5 Standard Edition" ON)
  option (BUILD_TEAMTALK_LIBRARY_UNITTEST_DOTNET_PRO "Build TeamTalk 5 Pro Unit Test for TeamTalk 5 Professional Edition" ON)

  if (BUILD_TEAMTALK_LIBRARY_UNITTEST_DOTNET)
    add_library (TeamTalk5Test.NET SHARED TeamTalkTest.cs Properties/AssemblyInfo.cs)
    set_property (TARGET TeamTalk5Test.NET PROPERTY VS_DOTNET_REFERENCES
      "Microsoft.VisualStudio.QualityTools.UnitTestFramework"
      "System" "System.Core" "System.Drawing")
    set_property(TARGET TeamTalk5Test.NET PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../Library/TeamTalk_DLL")
    target_link_libraries (TeamTalk5Test.NET TeamTalk5.NET)
  endif()
endif()
