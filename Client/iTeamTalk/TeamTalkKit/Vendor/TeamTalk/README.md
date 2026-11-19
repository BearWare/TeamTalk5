# TeamTalk C-API header files and libraries

TeamTalk.h contains the API for creating a TeamTalk client
application.

TeamTalkSrv.h contains the API for creating a TeamTalk server
application. The TeamTalk server API is only available in [TeamTalk 5
Professional Edition](http://bearware.dk/?page_id=419).

To get the TeamTalk5 and TeamTalk5Pro DLLs (shared libraries) download
the [TeamTalk 5 SDK](http://bearware.dk/?page_id=419). Alternatively
manually build `Library/TeamTalkLib` from the repository root. For iOS,
`make -C Build ios-all` produces the device and simulator static archives that
can be repackaged into `TeamTalkNativeiOS.xcframework`; the vendored iOS
XCFramework currently contains an `arm64` device slice and a universal
`x86_64`/`arm64` simulator slice built from `libTeamTalk5-simulator.a`.
