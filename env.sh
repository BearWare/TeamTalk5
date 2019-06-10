#!/bin/bash

export TEAMTALK_ROOT=$PWD

if [ "Darwin" = `uname -s` ]; then
    export DYLD_LIBRARY_PATH=$TEAMTALK_ROOT/Library/TeamTalk_DLL:$TEAMTALK_ROOT/Library/TeamTalkJNI/libs
else
    export LD_LIBRARY_PATH=$TEAMTALK_ROOT/Library/TeamTalk_DLL:$TEAMTALK_ROOT/Library/TeamTalkJNI/libs
fi

