# karabo-bridge-cpp

*karabo-bridge-cpp* is a C++ client to receive pipeline data from the
Karabo control system used at [European XFEL](https://www.xfel.eu/).

Under development!

## Building and installing

### zeromq

Remove libzmq-dev which has a zmq version of 2.2.0, if you have it installed.
```sh
sudo apt-get remove libzmq-dev
```
Download and install zeromq
```sh
wget https://github.com/zeromq/libzmq/releases/download/v4.2.5/zeromq-4.2.5.zip
unzip zeromq-4.2.5.zip
cd zeromq-4.2.5

./configure
make
make check  # optional
sudo make install
```

### msgpack

Download and install msgpack
```sh
wget https://github.com/msgpack/msgpack-c/releases/download/cpp-2.1.5/msgpack-2.1.5.tar.gz
tar -xzvf msgpack-2.1.5.tar.gz
cd msgpack-2.1.5

cmake -DMSGPACK_CXX11=ON .
sudo make install
```

## Usage



