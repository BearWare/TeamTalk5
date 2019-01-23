#!/bin/bash

if [ "Darwin" = `uname -s` ]; then
    export DYLD_LIBRARY_PATH=$PWD/Library/TeamTalk_DLL:$PWD/Library/TeamTalkJNI/libs
else
    export LD_LIBRARY_PATH=$PWD/Library/TeamTalk_DLL:$PWD/Library/TeamTalkJNI/libs
fi

