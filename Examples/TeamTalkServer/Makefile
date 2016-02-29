TEAMTALK_LIB=../../TeamTalk_DLL
TEAMTALK_INCLUDE=$(TEAMTALK_LIB)

help:
	@echo "Targets:"
	@echo "\tall"
	@echo "\t\t" "Build server for TeamTalk 5 SDK Professional Edition (libTeamTalk5Pro.so)"

all:
	g++ Main.cpp -I$(TEAMTALK_INCLUDE) -L$(TEAMTALK_LIB) -lTeamTalk5Pro -o ttproserver
