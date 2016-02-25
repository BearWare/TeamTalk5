
TEAMTALKLIB=../../../TeamTalkJNI/src/build/TeamTalk5.jar

help:
	@echo "Targets:"
	@echo "  all"
	@echo "    - Build Java TeamTalk server"
	@echo "  run"
	@echo "    - Run Java TeamTalk server"
	@echo "  clean"
	@echo "    - Clean build"
	@echo ""
	@echo "Ensure TeamTalk jar file exists here: $(TEAMTALKLIB)"

all:
	cd src && javac -cp $(TEAMTALKLIB):. TeamTalkServer.java

run:
	cd src && java -cp $(TEAMTALKLIB):. TeamTalkServer

clean:
	cd src && rm TeamTalkServer.class
