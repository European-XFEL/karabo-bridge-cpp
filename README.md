# karabo-bridge-cpp

[![Build Status](https://travis-ci.org/European-XFEL/karabo-bridge-cpp.svg?branch=master)](https://travis-ci.org/European-XFEL/karabo-bridge-cpp)

*karabo-bridge-cpp* provides a C++ interface to receive pipeline data from the Karabo control system used at [European XFEL](https://www.xfel.eu/).

## Dependencies

 - [ZeroMQ](http://zeromq.org/) >= 4.3.1
 - [cppzmq](https://github.com/zeromq/cppzmq) >= 4.2.5
 - [msgpack](https://msgpack.org/index.html) >= 3.2.0

## Deployment

### Build and install

```shell script
$ module load exfel exfel_anaconda3

# install dependencies
$ conda install -c anaconda cmake
$ conda install -c omgarcia gcc-6

$ conda install -c conda-forge cppzmq msgpack-c

# install karabo-bridge-cpp
$ git clone https://github.com/European-XFEL/karabo-bridge-cpp.git
$ cd karabo-bridge-cpp
$ mkdir build && cd build
$ cmake ../ && make
$ make install

```

### Unit test

```sh
$ # mkdir build && cd build
$ cmake -DBUILD_TESTS=ON ../ && make
$ make test
```

### Integration test

There are two ways to run the integration test:

1. *Integration test in two steps*

First, start the simulated server implemented in [karabo-bridge-py](https://github.com/European-XFEL/karabo-bridge-py):

```sh
$ karabo-bridge-server-sim 1234 -n 2
```

Then run the client in another terminal:

```sh
$ # mkdir build && cd build
$ cmake -DBUILD_INTEGRATION_TEST=ON ../ && make
$ make integration_test
```

2. *Integration test using Docker-compose*

```sh
# set up
$ sudo docker-compose up --build
# tear down
$ sudo docker-compose down
```

## Include the library in your own code

`karabo-bridge-cpp` is **not** a header only library! You need both `kb_client.hpp` and the dependencies to build your own code. An example `CMakeLists.txt` can be found in the folder `examples/smlt_camera`.

## Usage

```c++
import "kb_client.hpp"

karabo_bridge::Client client;
client.connect("tcp://localhost:1234")
```

#### showNext()

Use `showNext()` member function to return a string which tells you the data structure of the received multipart message.

*Note: this member function consumes data!*

You will see something like

```md
source: SPB_DET_AGIPD1M-1/DET/detector-1
Total bytes received: 402656475

path, container, container shape, type

metadata
--------
source, , , string
timestamp, , , double
timestamp.frac, , , string
timestamp.sec, , , string
timestamp.tid, , , uint64_t

data   # normal data
----
detector.data, array-like, [416], char
header.dataId, , , uint64_t
header.pulseCount, , , uint64_t
image.passport, array-like, [3], string

array   # Big chunk of data
-----
image.data, array-like, [16, 128, 512, 64], float
image.gain, array-like, [16, 128, 512, 64], uint16_t
```

#### next()

Use `next()` member function to return a `std::map<std::string, karabo_bridge::kb_data>`, where the key is the name of the data source and the value is a `kb_data` struct containing `metadata`, `data` and `array`. Each of them is a `std::map<std::string, object>`. It should be noted that "objects" in `metadata`, `data` and `array` are different.

##### metadata
Each "object" in `metadata` is a scalar data and can be visited via
```c++
uint64_t timestamp_tid = kb_data.metadata["timestamp.tid"].as<uint64_t>();
std::string timestamp.frac = kb_data.metadata["timestamp.frac"].as<std::string>();
```

##### data
Each "object" in `data` can be either a scalar data or an "array-like" data. It can be visited directly via
```c++
uint64_t header_pulsecount = kb_data["header.pulseCount"].as<uint64_t>();
```
for scalar data and
```c++
// Different containers are supported
std::deque<std::string> image_passport = kb_data["image.passport"].as<std::deque<std::string>>();
std::array<std::string, 3> image_passport = kb_data["image.passport"].as<std::array<std::string>, 3>();
std::vector<uint8_t> detector_data = kb_data["detector.data"].as<std::vector<uint8_t>>();
```
for "array-like" data.

To iterate over `data`, you can also use `kb_data` as a proxy. Both iterators and the range based for loop are supported. For example
```c++
for (auto it = kb_data.begin(); it != kb_data.end(); ++it) {}
for (auto& v : kb_data) {}
```


##### array
Each "object" in `array` is an "array-like" data. It can be visited via
```c++
// Different containers are supported
std::vector<float> image_data = kb_data.array["image.data"].as<std::vector<float>>();
std::deque<float> image_data = kb_data.array["image.data"].as<std::deque<float>>();
std::array<float, 67108864> image_data = kb_data.array["image.data"].as<std::array<float, 67108864>>();
```
Since `array` holds big chunk of data, in order to avoid the copy when constructing a container, you can also access the "array-like" data via pointers, e.g.
```c++
float* ptr = kb_data.array["image.data"].data<float>();
void* ptr = kb_data.array["image.data"].data();
```
*Note: A strict type checking is applied to `array` when casting. Implicit type conversion is not allowed. You must specify the exact type in the template, e.g. for the above example, you are not allowed to put 'double' in the template.*

##### Member functions for "object"

"objects" in `metadata`, `data` and `array` share the following common interface:

- `std::string dtype()`
Return the type for a scalar data and the data type inside the array for an "array-like" data.

- `std::vector<std::size_t> shape()`
Return an empty vector for a scalar data and the shape of array for an "array-like" data.

- `std::size_t size()`
Return 0 for a scalar data and the number of elements for an "array-like" data.

Examples
```c++
assert(kb_data["header.pulseCount"].dtype() == "uint64_t");
assert(kb_data["header.pulseCount"].shape().empty());
assert(kb_data["header.pulseCount"].size() == 0);
 
assert(kb_data.array["image.data"].dtype() == "float");
assert(kb_data.array["image.data"].shape()[0] == 16);
assert(kb_data.array["image.data"].shape()[1] == 128);
assert(kb_data.array["image.data"].shape()[2] == 512);
assert(kb_data.array["image.data"].shape()[3] == 64);
assert(kb_data.array["image.data"].size() == 16*128*512*64);
```
