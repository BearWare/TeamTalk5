#!/bin/bash

if [ "Darwin" = `uname -s` ]; then
    export DYLD_LIBRARY_PATH=$PWD/Library/TeamTalk_DLL:$PWD/Library/TeamTalkJNI/libs
else
    export LD_LIBRARY_PATH=$PWD/Library/TeamTalk_DLL:$PWD/Library/TeamTalkJNI/libs
fi

#locations of 'hamcrest-core' and 'junit'
export HAMCRESTCORE_JAR=$PWD/Library/TeamTalkJNI/hamcrest-core-1.3.jar
export JUNIT_JAR=$PWD/Library/TeamTalkJNI/junit-4.11.jar
