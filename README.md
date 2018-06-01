# karabo-bridge-cpp

*karabo-bridge-cpp* is a C++ client to receive pipeline data from the Karabo control system used at [European XFEL](https://www.xfel.eu/).

## Requirements

 - [ZeroMQ](http://zeromq.org/) >= [4.2.5](https://github.com/zeromq/libzmq/releases/download/v4.2.5/zeromq-4.2.5.zip)
 - [cppzmq](https://github.com/zeromq/cppzmq) >= [4.2.2](https://github.com/zeromq/cppzmq/archive/v4.2.2.zip)
 - [msgpack](https://msgpack.org/index.html) >= [2.1.5](https://github.com/msgpack/msgpack-c/releases/download)

## Set up the environment

#### Karabo-bridge-cpp

```sh
git clone https://github.com/European-XFEL/karabo-bridge-cpp.git
```

#### ZeroMQ

If you have "sudoer", e.g. in your own PC, then
```sh
sudo sh -c "echo 'deb http://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-stable/xUbuntu_16.04/ /' > /etc/apt/sources.list.d/network:messaging:zeromq:release-stable.list"
sudo apt-get update
sudo apt-get install libzmq3-dev
```

If you are on a cluster, then

```sh
./configure -prefix=$HOME/share/zeromq
make
make install
```

#### cppzmq
Copy the header files (`zmq.hpp`, `zmq_addon.hpp`) to `karabo-bridge-cpp/external/cppzmq/`.

#### msgpack
Copy the `include` folder to `karabo-bridge-cpp/external/msgpack/`.

#### Compiler
Make sure you have `g++ 4.8.5` installed in your own PC to be in line with the compiler in the Maxwell cluster.

## Build and test

```sh
./autogen.sh
```

## Run the examples

- For [example1](./src/client_for_pysim.cpp), you will need to have a Python simulated server running in a Screen:

```py
from karabo_bridge import start_gen
start_gen(1234)
```

then

```sh
build/run1
```

- For [example2](./src/client_for_smlt_camera.cpp), you will need to set up a Karabo device server with LimaSimulatedCamera and PipeToZeroMQ devices, then

```sh
build/run2
```

## How to use

```c++
import "kb_client.hpp"

karabo_bridge::Client client;
client.connect("tcp://localhost:1234")
```

#### showMsg()

Use `showMsg()` member function returns a string which tells you the multipart messsage structure.

*Note: this member function consumes data!*

You will see something like
```md

----------new message----------

"content": "msgpack",
"source": "camera:output"

----------new message----------
...
"data.image.geometry.alignment.offsets": [0,0,0],
"data.image.encoding": "GRAY",
"data.image.bitsPerPixel": 32,
...
"metadata.source": "camera:output",
"data.image.dimensions": [1024,1024],
"data.image.dimensionTypes": [0,0]

----------new message----------

"content": "array",
"dtype": "uint32",
"source": "camera:output",
"path": "data.image.data",
"shape": [1024,1024]

----------new message----------
0  # A random integer following a message containing the Array/ImageData header indicates a chunk of byte stream.

```

#### showNext()

Use `showNext()` member function to return a string which tells you the data structure of the received multipart message.

*Note: this member function consumes data!*

You will see something like

```md
data.image.bitsPerPixel: uint64_t
data.image.dimensionScales: string
data.image.dimensionTypes: msgpack::ARRAY, uint64_t, [2]
...
data.image.header: NIL
data.image.rOIOffsets: msgpack::ARRAY, uint64_t, [2]
metadata.ignored_keys: msgpack::ARRAY, NIL, [0]
metadata.source: string
metadata.timestamp: double
metadata.timestamp.frac: string
metadata.timestamp.sec: string
metadata.timestamp.tid: uint64_t
data.image.data: Array, uint32, [1024, 1024]
Total bytes received: 67111554
```

#### next()

Use `next()` member function to return a `karabo_bridge::kb_data` object
```c++
struct kb_data {
    std::map<std::string, Object> msgpack_data;
    std::map<std::string, Array> array;
    
    Object& operator[](const std::string& key) { return msgpack_data.at(key); }
    
    std::size_t size();
};

karabo_bridge::kb_data result = client.next();
```
You can visit the data members by
```c++
// Access directly the data member `msgpack_data` 
auto pulseCount = result["data.image.bitsPerPixel"].as<uint64_t>()
auto dataImageDimension = result["data.image.dimensions"].as<std::vector<uint64_t>>()

// Access the data member `array` which is the "array" or "ImageData" represented by char arrays
// Note:: you are responsible to give the correct data type, otherwise it leads to undefined behavior!
std::vector<uint64_t> imageData = result.array["data.image.data"].as<uint64_t>()
```

