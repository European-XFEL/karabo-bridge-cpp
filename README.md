# karabo-bridge-cpp

*karabo-bridge-cpp* is a C++ client to receive pipeline data from the Karabo control system used at [European XFEL](https://www.xfel.eu/).

## Requirements

 - [ZeroMQ](http://zeromq.org/) >= [4.2.5](https://github.com/zeromq/libzmq/releases/download/v4.2.5/zeromq-4.2.5.zip)
 - [cppzmq](https://github.com/zeromq/cppzmq) >= [4.2.2](https://github.com/zeromq/cppzmq/archive/v4.2.2.zip)
 - [msgpack](https://msgpack.org/index.html) >= [2.1.5](https://github.com/msgpack/msgpack-c/archive/cpp-2.1.5.zip)

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

- For [example1](./src/client_for_pysim.cpp), you will need to have a Python simulated server ([karabo-bridge-py]()) running in the background (e.g. a Screen session):

```py
from karabo_bridge import start_gen
start_gen(1234, nsources=2)
```

then

```sh
build/run1
```

## How to use

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

Metadata
--------
source, , , string
timestamp, , , double
timestamp.frac, , , string
timestamp.sec, , , string
timestamp.tid, , , uint64_t

Data   # normal data
----
detector.data, array-like, [416], char
header.dataId, , , uint64_t
header.linkId, , , uint64_t
header.majorTrainFormatVersion, , , uint64_t
header.minorTrainFormatVersion, , , uint64_t
header.pulseCount, , , uint64_t
header.reserved, array-like, [16], char
header.trainId, , , uint64_t
image.passport, array-like, [3], string
trailer.status, , , uint64_t

Array or ImageData  # Big chunk of data
------------------
image.data, array-like, [16, 128, 512, 64], float
image.gain, array-like, [16, 128, 512, 64], uint16_t
```

#### next()

Use `next()` member function to return a `std::map<std::string, karabo_bridge::kb_data>`, where the key is the name of the data source and `kb_data` is a struct containing `metadata`, `data` and `array`, where `array` refers to big chunk of data.

##### metadata
`metadataYou` is a `std::map` data structure. You can visit its element by
```c++
uint64_t timestamp_tid = kb_data.metadata["timestamp.tid"].as<uint64_t>();
std::string timestamp.frac = kb_data.metadata["timestamp.frac"].as<std::string>();
```

##### data
Elements in `data` can be visited by
```c++
uint64_t header_pulsecount = kb_data["header.pulseCount"].as<uint64_t>();

std::deque<std::string> image_passport = kb_data["image.passport"].as<std::deque<std::string>>();
std::array<std::string, 3> image_passport = kb_data["image.passport"].as<std::array<std::string>, 3>();
std::vector<uint8_t> detector_data = kb_data["detector.data"].as<std::vector<uint8_t>>();
```
You can also iterate over data through iterator, i.e. `kb_data.begin()`, `kb_data.end()` or the range based for loop.


##### array
`array` is also a `std::map` data structure. You can visit its element by
```c++
std::vector<float> image_data = kb_data.array["image.data"].as<std::vector<float>>();
std::deque<float> image_data = kb_data.array["image.data"].as<std::deque<float>>();
std::array<float, 67108864> image_data = kb_data.array["image.data"].as<std::array<float, 67108864>>();
```
You can also visit `array` via pointer, which can avoid the copy when constructing a container.
```c++
float* ptr = kb_data.array["image.data"].data<float>();
void* ptr = kb_data.array["image.data"].data();
```
*Note: A strict type checking is applied to `array` when casting. Implicit type conversion is not allowed. You must specify the exact type in the template, e.g. for the above example, you are not allowed to put 'double' in the template.*

##### Common inteface for `metadata`, `data` and `array`

Several useful member functions are implemented for all the data types

- `shape()`
Return the shape of the data as a vector. For scalar data, it returns an empty vector.

- `size()`
Return the size of a flatten array. For scalar data, it returns zero (to distinguish between a scalar data and an array of size 1).

- `dtype()`
Return the data type as a string. For "array-like" data, it returns the data type inside the container.

Examples
```c++
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
build/glimpse tcp://localhost:1234
```
To show the message structure:
```sh
build/glimpse tcp://localhost:1234 m
```

