# karabo-bridge-cpp

*karabo-bridge-cpp* is a C++ client to receive pipeline data from the
Karabo control system used at [European XFEL](https://www.xfel.eu/).

Under development!

## Building and installing

Download and install [zeromq](http://zeromq.org/)
```sh
wget https://github.com/zeromq/libzmq/releases/download/v4.2.5/zeromq-4.2.5.zip
unzip zeromq-4.2.5.zip
cd zeromq-4.2.5

./autogen.sh
./configure --prefix=/usr
make
make check  # optional
sudo make install
```

Download and install [cppzmq](https://github.com/zeromq/cppzmq)
```sh
wget https://github.com/zeromq/cppzmq/archive/v4.2.2.zip
unzip v4.2.2.zip
cd cppzmq-4.2.2

mkdir build
cd build
cmake ..
sudo make install
```


Download and install [msgpack](https://msgpack.org/index.html)
```sh
wget https://github.com/msgpack/msgpack-c/releases/download/cpp-2.1.5/msgpack-2.1.5.tar.gz
tar -xzvf msgpack-2.1.5.tar.gz
cd msgpack-2.1.5

cmake -DMSGPACK_CXX11=ON .
sudo make install
```

It is suggested to include the header file `include/kb_client.hpp` in your project and compile.

## Usage

```c++
import "kb_client.hpp"

karabo_bridge::Client client;
client.connect("tcp://localhost:1234")
```

#### showMsg()

Use `showMsg()` member function to write the multipart messsage structure into a file (default is `multipart_message.txt`).

*Note: this member function consumes data!*

In the file, you will see something like
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
0

```

#### showNext()

Use `showNext()` member function to write the data structure of the received multipart message into a file (default is `data_structure.txt`).

*Note: this member function consumes data!*

In the file, you will see something like

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
```

#### next()

Use `next()` member function to return a `karabo_bridge::kb_data` object
```c++
struct kb_data {
    std::map<std::string, Object> msgpack_data;
    std::map<std::string, Array> array;
    
    Object& operator[](const std::string& key) { return msgpack_data.at(key); }
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

## Examples

[example1](./src/client_for_pysim.cpp)
[example2](./src/client_for_smlt_camera.cpp)
