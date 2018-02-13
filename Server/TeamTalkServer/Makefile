TEAMTALK_LIB=../../TeamTalk_DLL
TEAMTALK_INCLUDE=$(TEAMTALK_LIB)

OS=$(shell uname)

ifeq ($(OS),Darwin)
SHARED_EXT := dylib
LIBPATHNAME=DYLD_LIBRARY_PATH
else
SHARED_EXT := so
LIBPATHNAME=LD_LIBRARY_PATH
endif

help:
	@echo "Targets:"
	@echo "  all"
	@echo "    - Build server for TeamTalk 5 SDK Professional Edition (libTeamTalk5Pro.$(SHARED_EXT))"
	@echo ""
	@echo "When running an application which uses the TeamTalk 5 SDK then"
	@echo "ensure $(LIBPATHNAME) is set up with the path to both "
	@echo "TeamTalk 5 DLL dependencies."
	@echo ""
	@echo "For TeamTalk 5 SDK Standard Edition i.e. libTeamTalk5.$(SHARED_EXT)"
	@echo "For TeamTalk 5 SDK Professional Edition i.e. libTeamTalk5Pro.$(SHARED_EXT)"
	@echo ""
	@echo "Example:"
	@echo "$$ export $(LIBPATHNAME)=SDK_ROOT/TeamTalk_DLL"

all:
	g++ Main.cpp -I$(TEAMTALK_INCLUDE) -L$(TEAMTALK_LIB) -lTeamTalk5Pro -o ttproserver
