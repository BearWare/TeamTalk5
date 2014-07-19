# TeamTalk 5 SDK

Repository for TeamTalk 5 SDK

Current version: [5.0.0.3201](http://www.bearware.dk/test/TeamTalk5SDK/v5.0.0.3201)

* **TeamTalk.NET**
  * TeamTalk 5 .NET DLL wrapper for C-API TeamTalk 5 DLL (**TeamTalk_DLL**)
    * Preliminary API [Documentation](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3201/docs/NET/)
* **TeamTalkAndroid**
  * TeamTalk 5 Android client application written in Java
  * Requires **TeamTalkJNI** project as library dependency
  * Import in Eclipse using [Android SDK](http://developer.android.com/sdk/index.html)
  * Copy the following files to **TeamTalkAndroid/libs** directory:
    * android-support-v4.jar
      * Located in {Eclipse ADT install-dir}/sdk/extras/android/support/v4
    * gson-2.2.4.jar
      * Download from http://code.google.com/p/google-gson/
* **TeamTalkApp.NET**
  * TeamTalk 5 .NET client application written in C#
  * Requires **TeamTalk.NET** project for DLL dependency
* **TeamTalkJNI**
  * TeamTalk 5 JNI project with Java wrapper classes
  * Import in Eclipse using [Android SDK](http://developer.android.com/sdk/index.html)
  * Requires ARM-v7a JNI shared object in sub-folder TeamTalkJNI/libs/armeabi-v7a
    * Download: [libTeamTalk5-jni.so](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3201/android/TeamTalk5JNI.tgz)
* **TeamTalk_DLL**
  * TeamTalk 5 C-API DLL project 
  * C-API header file for TeamTalk 5 DLL
    * Windows 32-bit
      * Import library: http://www.bearware.dk/test/TeamTalk5SDK/v5.0.0.3201/win32/TeamTalk5.lib
      * DLL: http://www.bearware.dk/test/TeamTalk5SDK/v5.0.0.3201/win32/TeamTalk5.dll
    * Windows 64-bit
      * Import library: http://www.bearware.dk/test/TeamTalk5SDK/v5.0.0.3201/win64/TeamTalk5.lib
      * DLL: http://www.bearware.dk/test/TeamTalk5SDK/v5.0.0.3201/win64/TeamTalk5.dll
    * Mac OS X 64-bit
      * DLL: http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3201/mac64/libTeamTalk5.dylib
    * Preliminary API [Documentation](http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3201/docs/C-API/)
* **qtTeamTalk**
  * TeamTalk 5 client application written in C++ and based on [Qt](http://www.qt-project.org)
  * Requires **TeamTalk_DLL** project for DLL dependency

## TeamTalk 5 Server binaries

* TeamTalk 5 console server
  * Windows 32-bit
    * http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3201/win32/tt5srv.exe
  * Windows 64-bit
    * http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3201/win64/tt5srv.exe
  * Debian i386
    * http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3201/deb32/tt5srv
  * Mac OS X 64-bit
    * http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3201/mac64/tt5srv

* TeamTalk 5 NT service
  * Windows 32-bit
    * http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3201/win32/tt5svc.exe
  * Windows 64-bit
    * http://bearware.dk/test/TeamTalk5SDK/v5.0.0.3201/win64/tt5svc.exe

