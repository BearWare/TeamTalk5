teamtalkkit-ios:
	xcodebuild -create-xcframework \
	  -library "$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5-arm64.a" \
	  -library "$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5-x86_64-simulator.a" \
	  -output  "$(TEAMTALK_ROOT)/Client/iTeamTalk/TeamTalkKit/Vendor/TeamTalkNativeiOS.xcframework"

teamtalkkit-ios-simulator:
	xcodebuild -create-xcframework \
	  -library "$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5-arm64-simulator.a" \
	  -library "$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5-x86_64-simulator.a" \
	  -output  "$(TEAMTALK_ROOT)/Client/iTeamTalk/TeamTalkKit/Vendor/TeamTalkNativeiOS.xcframework"

teamtalkkit-macos:
	xcodebuild -create-xcframework \
	  -library "$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5.a" \
	  -output  "$(TEAMTALK_ROOT)/Client/iTeamTalk/TeamTalkKit/Vendor/TeamTalkNativemacOS.xcframework"
