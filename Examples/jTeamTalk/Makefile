
TEAMTALKLIB=../../../TeamTalkJNI/src/build/TeamTalk5.jar

help:
	@echo "Targets:"
	@echo "  all"
	@echo "    - Build Java TeamTalk client"
	@echo "  run"
	@echo "    - Run TeamTalk Java client"
	@echo "  clean"
	@echo "    - Clean build"
	@echo ""
	@echo "Ensure TeamTalk jar file exists here: $(TEAMTALKLIB)"

all:
	cd src && javac -cp $(TEAMTALKLIB):. TeamTalkClient.java

run:
	cd src && java -cp $(TEAMTALKLIB):. TeamTalkClient

clean:
	cd src && rm TeamTalkClient.class
