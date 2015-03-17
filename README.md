# TeamTalk 5 SDK

Repository for [TeamTalk 5 SDK](http://www.bearware.dk/?page_id=393) development.

## Download TeamTalk 5 SDK

To build the TeamTalk client projects you must first download the
TeamTalk 5 SDK to obtain the client and server binaries.

* TeamTalk 5 SDK Standard Edition - **Beta** releases
  * [Windows 32-bit](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3896/tt5sdk_v5.0.0.3896_win32.zip) **rev. 3896**
  * [Windows 64-bit](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3896/tt5sdk_v5.0.0.3896_win64.zip) **rev. 3896**
  * [Mac 64-bit](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3896/tt5sdk_v5.0.0.3896_macos_x86_64.tar.gz) **rev. 3896**
  * [Debian 7 32-bit](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3896/tt5sdk_v5.0.0.3896_debian7_i386.tar.gz) **rev. 3896**
  * [Debian 7 64-bit](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3896/tt5sdk_v5.0.0.3896_debian7_x86_64.tar.gz) **rev. 3896**
  * [Raspberry Pi (armhf)](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3896/tt5sdk_v5.0.0.3896_raspbian_armhf.tar.gz) **rev. 3896**
  * [Android arm-v7](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3896/tt5sdk_v5.0.0.3896_android_armv7a.tar.gz)  **rev. 3896**
* TeamTalk 5 SDK Professional Edition - **Beta** releases
  * [Windows 32-bit](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3896/tt5prosdk_v5.0.0.3896_win32.zip)  **rev. 3896**
  * [Windows 64-bit](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3896/tt5prosdk_v5.0.0.3896_win64.zip)  **rev. 3896**
  * [Mac 64-bit](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3896/tt5prosdk_v5.0.0.3896_debian7_i386.tar.gz) **rev. 3896**
  * [Debian 7 32-bit](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3896/tt5prosdk_v5.0.0.3896_debian7_i386.tar.gz) **rev. 3896**
  * [Debian 7 64-bit](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3896/tt5prosdk_v5.0.0.3896_debian7_x86_64.tar.gz) **rev. 3896**
  * [Raspberry Pi](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3896/tt5prosdk_v5.0.0.3896_raspbian_armhf.tar.gz) **rev. 3896**

## TeamTalk 5 Libraries
Projects wrapping the client DLL file in the TeamTalk SDK.
* **TeamTalk_DLL**
  * TeamTalk 5 C-API DLL project 
  * C-API header file for TeamTalk 5 DLL
    * Preliminary API [Documentation](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3896/docs/C-API/)
* **TeamTalk.NET** (dependency: **TeamTalk_DLL**)
  * TeamTalk 5 .NET DLL wrapper for C-API TeamTalk 5 DLL (**TeamTalk_DLL**)
    * Preliminary API [Documentation](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3896/docs/NET/)
  * Requires DLL file from **TeamTalk_DLL** project, either 32-bit or 64-bit
* **TeamTalkJNI**
  * TeamTalk 5 JNI project with Java wrapper classes
  * Import in Eclipse using [Android SDK](http://developer.android.com/sdk/index.html)
  * Requires ARM-v7a JNI shared object in sub-folder *TeamTalkJNI/libs/armeabi-v7a*
    * Based on Android API Level 16
  * The following features are currently *not* supported in the JNI API:
    * Video capture (webcam)
    * Media file streaming

## TeamTalk 5 Clients
Projects containing client applications which use the TeamTalk 5 client DLL.
* **qtTeamTalk** (dependency: **TeamTalk_DLL**)
  * TeamTalk 5 client application written in C++ and based on [Qt](http://www.qt-project.org)
  * Requires **TeamTalk_DLL** project for DLL dependency
* **TeamTalkClassic** (dependency: **TeamTalk_DLL**)
  * TeamTalk 5 accessible client application written in C++ and based on MFC
    * Works well with screen-readers
  * Requires [Tolk](https://github.com/dkager/tolk) project as dependency. Remove macro *ENABLE_TOLK* to disable Tolk.
  * Requires **TeamTalk_DLL** project for DLL dependency
* **TeamTalkApp.NET** (dependency: **TeamTalk.NET**)
  * TeamTalk 5 .NET client application written in C#
  * Requires **TeamTalk.NET** project for DLL dependency
* **TeamTalkAndroid** (dependency: **TeamTalkJNI**)
  * TeamTalk 5 Android client application written in Java
  * Import project in Eclipse using [Android SDK](http://developer.android.com/sdk/index.html)
    * ... or build using [ant](http://ant.apache.org), run the following command: ```android update project -p . -s -t android-17```
  * Copy the following files to *TeamTalkAndroid/libs* directory:
    * android-support-v4.jar
      * Located in {Eclipse ADT install-dir}/sdk/extras/android/support/v4
    * android-support-v13.jar
      * Located in {Eclipse ADT install-dir}/sdk/extras/android/support/v13
    * gson-2.2.4.jar
      * Download from http://code.google.com/p/google-gson/
  * Requires **TeamTalkJNI** project as library dependency
* **ttphpadmin**
  * Console PHP-script for administrating a TeamTalk 5 server.
