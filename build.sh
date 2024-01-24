#!/bin/sh -l

PROJDIR="$(pwd)"
SOURCEDIR="$PROJDIR/src"
TESTDIR="$PROJDIR/tests"
BUILD_SCRIPT_DIR="$PROJDIR/build"

CFLAGS="-Wall -Werror -D_DEFAULT_SOURCE -D_GNU_SOURCE -DCOY_PROFILE -std=c11 -I$SOURCEDIR -I$TESTDIR"
LDLIBS="-lm -lpthread"

if [ "$#" -gt 0 -a "$1" = "debug" ]
then
    echo "debug build"
    CFLAGS="$CFLAGS -O0 -g"
elif [ "$#" -gt 0 -a "$1" != "clean" -o \( "$#" = 0 \) ]
then
    echo "release build"
    CFLAGS="$CFLAGS -O3"
fi

if [ "$#" -gt 0 -a "$1" = "clean" ] 
then
    echo "clean compiled programs"
    echo
    rm test $BUILD_SCRIPT_DIR/build $BUILD_SCRIPT_DIR/*.h
else
    cd $BUILD_SCRIPT_DIR
    cc build.c -o build
    ./build
    cd ..
    cc $CFLAGS $TESTDIR/test.c -o test $LDLIBS
fi

if [ "$#" -gt 0 -a "$1" = "test" ]
then
    ./test
fi

