TEAMTALK_LIB=../../TeamTalk_DLL
TEAMTALK_INCLUDE=$(TEAMTALK_LIB)

help:
	@echo "Targets:"
	@echo "\tpro"
	@echo "\t\t" "Build server for TeamTalk5Pro DLL"

pro:
	g++ Main.cpp -I$(TEAMTALK_INCLUDE) -L$(TEAMTALK_LIB) -lTeamTalk5Pro -o ttproserver
