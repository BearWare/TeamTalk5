#!/bin/bash

# Find absolute path (that works on both macOS and Linux :( )
CURDIR=$PWD
TEAMTALK_ROOT=$(dirname ${BASH_SOURCE[0]})
cd $TEAMTALK_ROOT
TEAMTALK_ROOT=$(pwd -P)
cd $CURDIR

PYTHONPATH=$TEAMTALK_ROOT/Library/TeamTalkPy:$PYTHONPATH
export TEAMTALK_ROOT PYTHONPATH

if [ "Darwin" = `uname -s` ]; then
    export DYLD_LIBRARY_PATH=$TEAMTALK_ROOT/Library/TeamTalk_DLL:$TEAMTALK_ROOT/Library/TeamTalkJNI/libs
else
    export LD_LIBRARY_PATH=$TEAMTALK_ROOT/Library/TeamTalk_DLL:$TEAMTALK_ROOT/Library/TeamTalkJNI/libs
fi
