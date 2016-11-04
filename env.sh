#!/bin/bash

if [ "Darwin" = `uname -s` ]; then
    export DYLD_LIBRARY_PATH=$PWD/TeamTalk_DLL:$PWD/TeamTalkJNI/libs
else
    export LD_LIBRARY_PATH=$PWD/TeamTalk_DLL:$PWD/TeamTalkJNI/libs
fi

#locations of 'hamcrest-core' and 'junit'
export HAMCRESTCORE_JAR=$PWD/TeamTalkJNI/hamcrest-core-1.3.jar
export JUNIT_JAR=$PWD/TeamTalkJNI/junit-4.11.jar
