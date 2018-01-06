cmake_minimum_required(VERSION 3.8)
project (TeamTalkApp.NET LANGUAGES CSharp)

include (CSharpUtilities)

set (APP_FORMS_SOURCES
  AudioStorageDlg.cs
  ChannelDlg.cs
  ConnectDlg.cs
  DesktopDlg.cs
  DesktopShareDlg.cs
  FileTransferDlg.cs
  HotKeyDlg.cs
  InputBox.cs
  LoginDlg.cs
  MainForm.cs
  MediaFileDlg.cs
  MessageDlg.cs
  PreferencesDlg.cs
  ServerPropertiesDlg.cs
  ServerStatsDlg.cs
  UserAccountsDlg.cs
  UserInfoDlg.cs
  VideoDlg.cs)

set (APP_DESIGNER_SOURCES
  Properties/AssemblyInfo.cs
  Properties/Settings.settings
  Properties/Resources.resx
  
  AudioStorageDlg.Designer.cs
  ChannelDlg.Designer.cs
  ConnectDlg.Designer.cs
  DesktopDlg.Designer.cs
  DesktopShareDlg.Designer.cs
  FileTransferDlg.Designer.cs
  HotKeyDlg.Designer.cs
  InputBox.Designer.cs
  LoginDlg.Designer.cs
  MainForm.Designer.cs
  MediaFileDlg.Designer.cs
  MessageDlg.Designer.cs
  PreferencesDlg.Designer.cs
  Properties/Resources.Designer.cs
  Properties/Settings.Designer.cs
  ServerPropertiesDlg.Designer.cs
  ServerStatsDlg.Designer.cs
  UserAccountsDlg.Designer.cs
  UserInfoDlg.Designer.cs
  VideoDlg.Designer.cs  
  
  AudioStorageDlg.resx
  ChannelDlg.resx
  ConnectDlg.resx
  DesktopDlg.resx
  DesktopShareDlg.resx
  FileTransferDlg.resx
  HotKeyDlg.resx
  InputBox.resx
  LoginDlg.resx
  MainForm.resx
  MediaFileDlg.resx
  MessageDlg.resx
  PreferencesDlg.resx
  Properties/Resources.resx
  ServerPropertiesDlg.resx
  ServerStatsDlg.resx
  UserAccountsDlg.resx
  UserInfoDlg.resx
  VideoDlg.resx)

set (APP_SOURCES
  ${APP_FORMS_SOURCES}
  ${APP_DESIGNER_SOURCES}
  
  ChannelsView.cs
  FilesView.cs
  Program.cs
  UsersView.cs
  Util.cs

  
  # channel.png
  # connect.png
  # hotkey.png
  # user.png
  # webcam.png
  )


if (MSVC)
  option (BUILD_TEAMTALKCLIENT_DOTNET_EXAMPLE "Build TeamTalkApp.NET for TeamTalk 5 Standard Edition" ON)
  option (BUILD_TEAMTALKCLIENTPRO_DOTNET_EXAMPLE "Build TeamTalkProApp.NET for TeamTalk 5 Professional Edition" ON)

  csharp_set_windows_forms_properties (${APP_FORMS_SOURCES} ${APP_DESIGNER_SOURCES})
  
  if (BUILD_TEAMTALKCLIENT_DOTNET_EXAMPLE)
    add_executable (TeamTalk5App.NET WIN32 ${APP_SOURCES})
    set_property (TARGET TeamTalk5App.NET PROPERTY VS_DOTNET_REFERENCES
      "System" "System.Data" "System.Deployment" "System.Drawing" "System.Windows.Forms" "System.Xml")
    target_link_libraries (TeamTalk5App.NET TeamTalk5.NET)
  endif()

  if (BUILD_TEAMTALKCLIENTPRO_DOTNET_EXAMPLE)
    add_executable (TeamTalk5ProApp.NET WIN32 ${APP_SOURCES})
    set_property (TARGET TeamTalk5ProApp.NET PROPERTY VS_DOTNET_REFERENCES
      "System" "System.Data" "System.Deployment" "System.Drawing" "System.Windows.Forms" "System.Xml")
    target_compile_options (TeamTalk5ProApp.NET PUBLIC "/define:ENABLE_ENCRYPTION")
    target_link_libraries (TeamTalk5ProApp.NET TeamTalk5Pro.NET)
  endif()
endif()
