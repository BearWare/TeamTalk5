
TEAMTALKLIB=../../../TeamTalkJNI/libs/TeamTalk5.jar

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
	@echo "    - Build TeamTalk Java server"
	@echo "  run"
	@echo "    - Run TeamTalk Java server"
	@echo "  clean"
	@echo "    - Clean build"
	@echo ""
	@echo "Ensure TeamTalk jar file exists here: $(TEAMTALKLIB)"
	@echo ""
	@echo "When running an application which uses the TeamTalk 5 SDK then"
	@echo "ensure $(LIBPATHNAME) is set up with the path to both "
	@echo "TeamTalk 5 DLL dependencies."
	@echo ""
	@echo "For TeamTalk 5 SDK Professional Edition i.e. libTeamTalk5Pro.$(SHARED_EXT)"
	@echo "and libTeamTalk5Pro-jni.$(SHARED_EXT)"
	@echo ""
	@echo "Example:"
	@echo "$$ export $(LIBPATHNAME)=SDK_ROOT/TeamTalk_DLL:SDK_ROOT/TeamTalkJNI/libs"

all:
	cd src && javac -cp $(TEAMTALKLIB):. TeamTalkServer.java

run:
	cd src && java -cp $(TEAMTALKLIB):. TeamTalkServer

clean:
	cd src && rm TeamTalkServer.class
