#!/bin/bash

TEAMTALK_ROOT=$(dirname ${BASH_SOURCE[0]})
export TEAMTALK_ROOT

if [ "Darwin" = `uname -s` ]; then
    export DYLD_LIBRARY_PATH=$TEAMTALK_ROOT/Library/TeamTalk_DLL:$TEAMTALK_ROOT/Library/TeamTalkJNI/libs
else
    export LD_LIBRARY_PATH=$TEAMTALK_ROOT/Library/TeamTalk_DLL:$TEAMTALK_ROOT/Library/TeamTalkJNI/libs
fi
