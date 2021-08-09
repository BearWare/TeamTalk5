# Building WebRTC

WebRTC is not built automatically by CMake because it comes with its
own repository and tools. Quite a few manual steps are needed to setup
and build WebRTC.

### Build WebRTC for Windows

Start a Visual Studio 2019 command prompt, either "x86 Native Tools
Command Prompt for VS 2019" or "x64 Native Tools Command Prompt for VS
2019" depending on the architecture to build for.

In the command prompt first specify the location of the OpenSSL build
in an ```OPENSSL_DIR``` environment variable.

Run ```build_win.bat``` and follow the instructions.
