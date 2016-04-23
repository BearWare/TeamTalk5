# TeamTalk 5 Classic for Accessibility based on [MFC](https://msdn.microsoft.com/en-us/library/d06h2x6e.aspx)

To try a compiled version of this application go [here](http://bearware.dk/?page_id=367).

## Build TeamTalk 5 Classic without Tolk

By default the project file TeamTalkClassic.vcxproj builds with the ENABLE_TOLK macro enabled.
If you don't want to use [Tolk](https://github.com/dkager/tolk) then remove the ENABLE_TOLK
macro from C/C++ Preprocessor options.

## Build Tolk for TeamTalk 5 Classic

Either build using the instructions on the [Tolk](https://github.com/dkager/tolk) project or
use a forked version [here](https://github.com/bear101/tolk). If using the forked version
then start a Visual Studio console window and run the build_tolk_x86.bat and build_tolk_x64.bat
files.
