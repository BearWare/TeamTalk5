#!/bin/bash

# Find absolute path (that works on both macOS and Linux :( )
CURDIR=$PWD
ENVDIR="${BASH_SOURCE[0]}"
# zsh does not set BASH_SOURCE variable, so assume zsh if blank
if [ -z $ENVDIR ]; then
    echo "BASH_SOURCE is blank so using zsh syntax"
    ENVDIR=${0:a:h}
else
    ENVDIR=$(dirname $ENVDIR)
fi
TEAMTALK_ROOT=$ENVDIR
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
