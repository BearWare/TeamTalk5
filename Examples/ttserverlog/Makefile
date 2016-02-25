TEAMTALK_LIB=../TeamTalk_DLL
TEAMTALK_INCLUDE=$(TEAMTALK_LIB)

help:
	@echo "Targets:"
	@echo "\tstd"
	@echo "\t\t" "Build client for TeamTalk5 DLL"
	@echo "\tpro"
	@echo "\t\t" "Build client for TeamTalk5Pro DLL"

std:
	g++ Main.cpp -I$(TEAMTALK_INCLUDE) -L$(TEAMTALK_LIB) -lTeamTalk5 -o ttserverlog

pro:
	g++ Main.cpp -I$(TEAMTALK_INCLUDE) -L$(TEAMTALK_LIB) -lTeamTalk5Pro -o ttproserverlog
