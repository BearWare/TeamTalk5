# TeamTalk 5 client application based on [Qt Framework](http://www.qt.io)

Here are the instructions on how to build the TeamTalk 5 Qt client.

To try a compiled version of the this application go [here](http://bearware.dk/?page_id=327).

## Build Qt-based TeamTalk 5 on Ubuntu 18

In order to build the Qt-based TeamTalk client its build-dependencies
must first be installed. Goto TEAMTALK_ROOT/Build and type:

`# make depend-ubuntu64`

Afterwards run the following command:

`# qmake teamtalk5.pro`

Followed by:

`# make`

## Build Qt-based TeamTalk 5 on Windows

First use CMake to build Qt Framework:

`# cmake -S C:/TeamTalk5 -B qtbuild -A Win32 -DQT_BUILD_QTFRAMEWORK=ON -DQT_INSTALL_PREFIX=C:/Qt5`
`# cmake --build qtbuild`

Setup Qt Framework in PATH so we can use *qmake.exe* to generate
project files:

`# set PATH=C:\Qt5\bin;%PATH%`

Generate project files for TeamTalk Qt-based client:

`# qmake -tp vc CONFIG+=release teamtalk5.pro`

Now open *TeamTalk5.vcxproj* in Visual Studio 2019 and build.
