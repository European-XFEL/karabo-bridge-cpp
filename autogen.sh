rm -r build 2> /dev/null
mkdir build
cd build

if ! [ -x "$(command -v g++-4.8)" ]; then
    echo "g++4.8 does not exist! Compile with default g++!"
    cmake ..
else
    echo "Compile with g++4.8!"
    cmake -D CMAKE_CXX_COMPILER=/usr/bin/g++-4.8 ..
fi

make
make test
cd ..
