# TeamTalk 5

Repository for TeamTalk 5 development.

[![Build Status](https://travis-ci.org/BearWare/TeamTalk5.svg?branch=master)](https://travis-ci.org/BearWare/TeamTalk5)

## Overview
* [Download TeamTalk 5 SDK](#download-teamtalk-5-sdk)
  * Get the client and server binaries for building your own software
* [TeamTalk 5 Libraries](#teamtalk-5-libraries)
  * The libraries (APIs) that can be used for developing your client and server
* [TeamTalk 5 Clients](#teamtalk-5-clients)
  * Example projects that show how to build your own TeamTalk client
* [TeamTalk 5 Servers](#teamtalk-5-servers)
  * Example projects that show how to build your own TeamTalk server

## Download TeamTalk 5 SDK

To build the TeamTalk client or server projects you must first
download the [TeamTalk 5 SDK](http://www.bearware.dk/?page_id=393) to
obtain the client and server binaries.

You can also try out the latest beta releases of the TeamTalk
5 SDK [here](http://www.bearware.dk/test/teamtalksdk). Some of the
TeamTalk 5 SDK beta releases are only available for certain platforms,
so if your platform is not listed then try a previously released beta.

## TeamTalk 5 Libraries
Projects wrapping the client DLL file in the TeamTalk SDK.
* **TeamTalkLib**  (dependency: [TeamTalk toolchain](https://github.com/bear101/toolchain))
  * Source code for building TeamTalk 5 DLL and server executables
  * Read License.txt for terms of use
  * Build using [CMake](http://www.cmake.org)
* **TeamTalk_DLL** (dependency: **TeamTalkLib**)
  * TeamTalk 5 C-API DLL
  * C-API header files for TeamTalk 5 DLL
    * Preliminary API [Documentation](http://bearware.dk/test/teamtalksdk/docs/C-API/)
  * Build using TeamTalkLib or [download TeamTalk 5 SDK](https://bearware.dk/?page_id=419)
* **TeamTalk.NET** (dependency: **TeamTalk_DLL**)
  * TeamTalk 5 .NET DLL wrapper for C-API TeamTalk 5 DLL (**TeamTalk_DLL**)
    * Preliminary API [Documentation](http://bearware.dk/test/teamtalksdk/docs/NET/)
  * Requires DLL file from **TeamTalk_DLL** project, either 32-bit or 64-bit
* **TeamTalkJNI** (dependency: **TeamTalk_DLL**)
  * TeamTalk 5 JNI project with Java wrapper classes
    * Preliminary API [Documentation](http://bearware.dk/test/teamtalksdk/docs/Java/)
* **TeamTalkPy** (dependency: **TeamTalk_DLL**)
  * Python wrappers for TeamTalk DLL

## TeamTalk 5 Clients
Projects containing client applications which use the TeamTalk 5 client DLL.
* **qtTeamTalk** (dependency: **TeamTalk_DLL**)
  * TeamTalk 5 client application written in C++ and based on [Qt](http://www.qt.io)
  * Requires **TeamTalk_DLL** project for DLL dependency
* **TeamTalkClassic** (dependency: **TeamTalk_DLL**)
  * TeamTalk 5 accessible client application written in C++ and based on MFC
    * Works well with screen-readers
  * Requires [Tolk](https://github.com/dkager/tolk) project as dependency. Remove macro *ENABLE_TOLK* to disable Tolk.
    * Run ```git submodule init``` followed by ```git submodule update```
  * Requires [tinyxml](https://github.com/bear101/tinyxml) project as dependency.
    * Run ```git submodule init``` followed by ```git submodule update```
  * Requires **TeamTalk_DLL** project for DLL dependency
* **TeamTalkApp.NET** (dependency: **TeamTalk.NET**)
  * TeamTalk 5 .NET client application written in C#
  * Requires **TeamTalk.NET** project for DLL dependency
* **iTeamTalk** (dependency: **TeamTalk_DLL**, **FacebookSDK**)
  * TeamTalk 5 iOS client application written in Swift
  * Requires **TeamTalk_DLL** project for bridging header
  * Requires **[FacebookSDK](https://developers.facebook.com/docs/ios)**
  * Open project in [Xcode](https://developer.apple.com/xcode/)
* **TeamTalkAndroid** (dependency: **TeamTalkJNI**)
  * TeamTalk 5 Android client application written in Java
  * Open project in [Android Studio](https://developer.android.com/studio/intro/index.html)
  * Copy the TeamTalk 5 Java library to *TeamTalkAndroid/libs* directory:
    * TeamTalk5.jar
      * Located in ```Library/TeamTalkJNI/libs``` (Standard Edition)
    * TeamTalk5Pro.jar
      * Located in ```Library/TeamTalkJNI/libs``` (Professional Edition)
  * Copy the TeamTalk 5 JNI shared library to *TeamTalkAndroid/src/main/jniLibs/[armeabi-v7a|arm64-v8a|x86|x86_64]*
    * libTeamTalk5-jni.so
        * Located in ```Library/TeamTalkJNI/libs/[armeabi-v7a|arm64-v8a|x86|x86_64]``` (Standard Edition)
    * libTeamTalk5Pro-jni.so
        * Located in ```Library/TeamTalkJNI/libs/[armeabi-v7a|arm64-v8a|x86|x86_64]``` (Professional Edition)
* **ttserverlogpy** (dependency: **TeamTalkPy**
  * Python example client
* **ttphpadmin**
  * Console PHP-script for administrating a TeamTalk 5 server.
  * Run ```php ttphpadmin.php```

## TeamTalk 5 Servers
Sample applications for writing a TeamTalk 5 server are located in the Examples folder. Building a TeamTalk 5 server requires TeamTalk 5 Professional Edition.
* **TeamTalkServer** (dependency: **TeamTalk_DLL**)
  * TeamTalk 5 server application written in C++
  * Requires **TeamTalk_DLL** project for DLL dependency
* **TeamTalkServer.NET** (dependency: **TeamTalk.NET**)
  * TeamTalk 5 server application written in C#
  * Requires **TeamTalk_DLL** and **TeamTalk.NET** projects for DLL dependencies
* **jTeamTalkServer** (dependency: **TeamTalkJNI**)
  * TeamTalk 5 server application written in Java
  * Requires **TeamTalk_DLL** and **TeamTalkJNI** for DLL dependencies
