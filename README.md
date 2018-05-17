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

## Usage

```c++
import "kb_client.hpp"

karabo_bridge::Client client;
client.connect("tcp://localhost:1234")
```

Use `showNext()` member function to write the data structure into a file ("data_structure_from_server.txt").

*Note: this member function consumes data!*
```c++
client.showNext()
```
In the file, you will see something like
```md
"SPB_DET_AGIPD1M-1/DET/detector":
    ...
    "metadata":
        "timestamp":
            "frac": 847052,
            "tid": 10000000000,
            "sec": 1526209683,
        "source": "SPB_DET_AGIPD1M-1/DET/detector",
    "image.trainId":
        type: "<u8",
        shape: [32],
        nd: true,
        data: (bin),
        kind: (bin),
    "header.minorTrainFormatVersion": 1,
    "trailer.trainId": 10000000000,
    ...
```
Use `next()` member function to return a `karabo_bridge::data` object
```c++
struct data {
    std::string source;
    std::map<std::string, object> timestamp;
    std::map<std::string, object> data_;

    ...
};

karabo_bridge::data result = client.next();
```
You can visit the data by
```c++
// Access the item in member `timstamp`
auto tid = result.timestamp["tid"].as<uint64_t>()

// Access directly the item in member `data_`
auto pulse_count = result["header.pulseCount"].as<uint64_t>()

// Access the array packed as a "bin" (char array)
// Note:: you are responsible to give the correct data type and array size, otherwise it leads to undefined behavior!
std::array<uint64_t, 32> train_id = result["image.trainId"].asArray<uint64_t, 32>()
```
