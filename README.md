# karabo-bridge-cpp

*karabo-bridge-cpp* is a C++ client to receive pipeline data from the Karabo control system used at [European XFEL](https://www.xfel.eu/).

## Requirements

 - [ZeroMQ](http://zeromq.org/) >= 4.2.5
 - [cppzmq](https://github.com/zeromq/cppzmq) >= 4.2.2
 - [msgpack](https://msgpack.org/index.html) >= 2.1.5

## Set up the environment

#### Compiler
The Maxwell cluster uses *g++ 4.8.5*.

#### CMake
The Maxwell cluster uses *CMake 2.8.12.2*.

#### ZeroMQ

If you have "sudoer", e.g. in your own PC, then
```sh
$ sudo sh -c "echo 'deb http://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-stable/xUbuntu_16.04/ /' > /etc/apt/sources.list.d/network:messaging:zeromq:release-stable.list"
$ sudo apt-get update
$ sudo apt-get install libzmq3-dev
```

If you are on a cluster, then

```sh
$ ./configure -prefix=${HOME}/share/zeromq
$ make
$ make install
```

#### cppzmq

```sh
$ wget https://github.com/zeromq/cppzmq/archive/v4.2.2.tar.gz
$ tar -xzf v4.2.2.tar.gz
$ mkdir -p ${HOME}/share/cppzmq/include
$ cp cppzmq-4.2.2/*.hpp ${HOME}/share/cppzmq/include
```

#### msgpack

```sh
$ wget https://github.com/msgpack/msgpack-c/archive/cpp-2.1.5.tar.gz
$ tar -xzf cpp-2.1.5.tar.gz
$ mkdir -p ${HOME}/share/msgpack
$ cp -r msgpack-c-cpp-2.1.5/include ${HOME}/share/msgpack/
```

## Docker

We provide a Docker container with the above environment being set up.

```sh
$ sudo docker run -it zhujun98/maxwell bash
```

## Build and test

```sh
$ git clone https://github.com/European-XFEL/karabo-bridge-cpp.git
$ cd karabo-bridge-cpp
$ ./autogen.sh
```

## Run the examples

- For [example1](./src/client_for_pysim.cpp), you will need to have a Python simulated server ([karabo-bridge-py]()) running in the background:

```sh
$ karabo-bridge-server-sim 1234 -n 2
```

then

```sh
$ build/run1
```

## How to use

Include the header file `karabo-bridge-cpp/include/kb_client.hpp` in your code and build.

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

## Tools

#### glimpse

To show the data structure:
```sh
$ build/glimpse tcp://localhost:1234
```
To show the message structure:
```sh
$ build/glimpse tcp://localhost:1234 m
```

