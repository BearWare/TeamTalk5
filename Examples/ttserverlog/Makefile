TEAMTALK_LIB=../../TeamTalk_DLL
TEAMTALK_INCLUDE=$(TEAMTALK_LIB)

help:
	@echo "Targets:"
	@echo "\tstd"
	@echo "\t\t" "Build client for TeamTalk 5 SDK Standard Edition (libTeamTalk5.so)"
	@echo "\tpro"
	@echo "\t\t" "Build client for TeamTalk 5 SDK Professional Edition (libTeamTalk5Pro.so)"

std:
	g++ Main.cpp -I$(TEAMTALK_INCLUDE) -L$(TEAMTALK_LIB) -lTeamTalk5 -o ttserverlog

pro:
	g++ Main.cpp -I$(TEAMTALK_INCLUDE) -L$(TEAMTALK_LIB) -lTeamTalk5Pro -o ttproserverlog
