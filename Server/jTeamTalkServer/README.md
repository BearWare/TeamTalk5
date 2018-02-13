# TeamTalk 5 Java Server

Building the TeamTalk 5 Java server requires TeamTalk 5 SDK Professional Edition.

Library dependencies:

* **TeamTalk_DLL**
  * libTeamTalk5Pro.so
    * Linux
    * SDK path `Library/C-API/TeamTalk_DLL`
    * Put in environment variable `LD_LIBRARY_PATH`
  * libTeamTalk5Pro.dylib
    * Mac OS X
    * SDK path `Library/C-API/TeamTalk_DLL`
    * Put in environment variable `DYLD_LIBRARY_PATH`
  * TeamTalk5Pro.dll
    * Windows
    * SDK path `Library/C-API/TeamTalk_DLL`
    * Put in environment variable `PATH`
* **TeamTalkJNI**
  * libTeamTalk5Pro-jni.so
    * Linux
    * SDK path `Library/Java/TeamTalkJNI`
    * Put in environment variable `LD_LIBRARY_PATH`
  * libTeamTalk5Pro-jni.dylib
    * Mac OS X
    * SDK path `Library/Java/TeamTalkJNI`
    * Put in environment variable `DYLD_LIBRARY_PATH`
  * TeamTalk5Pro-jni.dll
    * Windows
    * SDK path `Library/Java/TeamTalkJNI`
    * Put in environment variable `PATH`
