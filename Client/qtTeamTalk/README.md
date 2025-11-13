# TeamTalk 5 client application based on [Qt Framework](http://www.qt.io)

Here are the instructions on how to build the TeamTalk 5 Qt client.

To try a compiled version of the this application go [here](http://bearware.dk/?page_id=327).

## Build Qt-based TeamTalk 5 on Ubuntu 22

In order to build the Qt-based TeamTalk client its build-dependencies
must first be installed. Goto TEAMTALK_ROOT/Build and type:

`# make depend-ubuntu22`

Afterwards run the following command to build:

`# cmake -S $HOME/TeamTalk5 -B builddir`
`# cmake --build builddir`

## Build Qt-based TeamTalk 5 on Windows

In order to build the Qt TeamTalk client first install Qt Framework
from https://www.qt.io/

Setup Qt Framework in environment variables so CMake can detect Qt
Framework, e.g.:

`# set QTDIR=c:\Qt\6.8.0\msvc2026_64`
`# set PATH=%QTDIR%\bin;%PATH%`

Now use CMake to generate project files:

`cmake -G "Visual Studio 18 2026" -A x64 -S C:/TeamTalk5 -B builddir`
`cmake --build builddir

To open the generated project files in Visual Studio 2026 open
*TeamTalk5.vcxproj*.

## Update Translation Files for Qt-based TeamTalk 5

Using the build setup described in the two above section then to
update translation files (.ts) run the following command:

`# cmake --build builddir --target update_translations`
