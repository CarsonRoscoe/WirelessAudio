#!/bin/bash

# ----- build.sh -----
# script to run windeployqt.exe automatically on Qt projects
# windeployqt copies over the DLLs required by the exe
#
# notes:
#   - meant to be run from within the root git repository
#   - assumes default Qt installation directory of C:\Qt\Qt5.x.x

TARGET="all"

while getopts ":cs" opt; do
  case $opt in
    c)
        TARGET="c"
        echo "-c building client..."
        ;;
    s)
        TARGET="s"
        echo "-c building server..."
        ;;
    \?)
        echo "Valid options: -c -s" >&2
        exit
        ;;
  esac
done

if [[ `find -name "build-ComAudio*" | wc -l` < 2 ]]; then
    echo "Missing build folders..." >&2
    exit
fi

serverpath=$(readlink -f ./build-ComAudioServer*/debug/ComAudioServer.exe)
clientpath=$(readlink -f ./build-ComAudioClient*/debug/ComAudioClient.exe)

if [[ $TARGET == "all" ]]; then
    echo "building client and server..."
    (exec "/c/Qt/Qt5.6.0/5.6/msvc2015_64/bin/windeployqt.exe" $clientpath)
    (exec "/c/Qt/Qt5.6.0/5.6/msvc2015_64/bin/windeployqt.exe" $serverpath)
elif [[ $TARGET == "c" ]]; then
    (exec "/c/Qt/Qt5.6.0/5.6/msvc2015_64/bin/windeployqt.exe" $clientpath)
elif [[ $TARGET == "s" ]]; then
    (exec "/c/Qt/Qt5.6.0/5.6/msvc2015_64/bin/windeployqt.exe" $serverpath)
fi
