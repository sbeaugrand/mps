#!/bin/bash
test "$XC" != "aarch64-linux-gnu" || ARCH=arm64
test "$XC" != "arm-linux-gnueabihf" || ARCH=armhf
if [ -n "$1" ]; then
    docker run -it --rm --volume=$PWD:/tmp/mps -w /tmp/mps -e XC=$XC debian-$ARCH $*
else
    docker run -it --rm --volume=$PWD:/tmp/mps -w /tmp/mps debian-$ARCH make xc XC=$XC CMAKE_OPT="-DDOCKER_XC=1 -DCMAKE_CXX_COMPILER=$XC-g++ -DCMAKE_C_COMPILER=$XC-gcc"
fi
