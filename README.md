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

#### showNext()

Use `showNext()` member function to write the multipart messsage structure into a file ("multipart_message_structure.txt").

*Note: this member function consumes data!*

In the file, you will see something like
```md
----------new message----------

"source": "SPB_DET_AGIPD1M-1/DET/detector",
"content": "array",
"path": "image.cellId",
"dtype": "uint16",
"shape": [32]

----------new message----------
0

----------new message----------

"source": "SPB_DET_AGIPD1M-1/DET/detector",
"content": "array",
"path": "image.length",
"dtype": "uint32",
"shape": [32]
```
#### next()

Use `next()` member function to return a `karabo_bridge::data` object
```c++
struct kb_data {
    std::map<std::string, Object> msgpack_data;
    std::map<std::string, ObjectBin> data;
    ...
};

karabo_bridge::data result = client.next();
```
You can visit the data by
```c++
// Access directly the data member `msgpack_data` 
auto pulseCount = result["header.pulseCount"].as<uint64_t>()

// Access the data member `data` which is the "array" or "ImageData" represented by char arrays
// Note:: you are responsible to give the correct data type, otherwise it leads to undefined behavior!
std::vector<uint64_t> train_id = result.data["image.trainId"].as<uint64_t>()
```
