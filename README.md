# karabo-bridge-cpp

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
unzip zeromq-4.2.5.zip && rm zeromq-4.2.5.zip
cd zeromq-4.2.5

./configure
make
make check  # optional
sudo make install
```

### msgpack

## Usage



