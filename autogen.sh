#!/bin/bash

INSTALL_PATH=${HOME}/share/karabo_bridge_cpp/
if [ $# -ge 1 ]; then
    INSTALL_PATH=$1
fi

rm -r build 2> /dev/null
mkdir build
cd build

if ! [ -x "$(command -v g++-4.8)" ]; then
    echo "g++4.8 does not exist! Compile with default g++!"
    cmake -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL_PATH} ..
else
    echo "Compile with g++4.8!"
    cmake -DCMAKE_CXX_COMPILER=/usr/bin/g++-4.8 -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL_PATH} ..
fi

make -j ${nproc}
make test
make install
cd ..
