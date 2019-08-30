#!/bin/bash

ZEROMQ_VERSION=4.3.1
CPPZMQ_VERSION=4.2.5
MSGPACK_VERSION=3.2.0

ZEROMQ_FOLDER=zeromq-${ZEROMQ_VERSION}
ZEROMQ_DOWNLOAD=${ZEROMQ_FOLDER}.tar.gz

CPPZMQ_FOLDER=cppzmq-${CPPZMQ_VERSION}
CPPZMQ_DOWNLOAD=v${CPPZMQ_VERSION}.tar.gz

MSGPACK_FOLDER=msgpack-c-cpp-${MSGPACK_VERSION}
MSGPACK_DOWNLOAD=cpp-${MSGPACK_VERSION}.tar.gz

show_usage()
{
    echo -e "\nUsage ./autogen.sh option [destination]"
    echo -e "\nValid options are:"
    echo -e " - download: download the dependencies;"
    echo -e " - build: build the library;"
    echo -e " - test: build and run unit test;"
    echo -e " - integration_test: build and run integration test (only client);"
    echo -e " - install: build and install to the destination folder, default to "${HOME}/share";"
    echo -e " - clean: remove the build directory and downloaded dependencies.\n"
}

maybe_download_zeromq()
{
    if [ ! -f ${ZEROMQ_DOWNLOAD} ]; then
        wget https://github.com/zeromq/libzmq/releases/download/v${ZEROMQ_VERSION}/${ZEROMQ_DOWNLOAD} || exit 1
    fi
}

maybe_download_cppzmq()
{
    if [ ! -f ${CPPZMQ_DOWNLOAD}  ]; then
        wget https://github.com/zeromq/cppzmq/archive/${CPPZMQ_DOWNLOAD} || exit 1
    fi
}

maybe_download_msgpack()
{
    if [ ! -f cpp-${MSGPACK_VERSION}.tar.gz ]; then
        wget https://github.com/msgpack/msgpack-c/archive/cpp-${MSGPACK_VERSION}.tar.gz || exit 1
    fi
}

if [ $# -eq 0 ]; then
    show_usage
    exit 1
fi

OPTION=$1

INSTALL_PATH=$2
if [ ! ${INSTALL_PATH} ]; then
    INSTALL_PATH=${HOME}/share
fi

TEST=false
INTEGRATION_TEST=false
INSTALL=false
if [ ${OPTION} == "download" ]; then
    maybe_download_zeromq
    maybe_download_cppzmq
    maybe_download_msgpack
    exit 0
elif [ ${OPTION} == "build" ]; then
    echo
elif [ ${OPTION} == "test" ]; then
    TEST=true
elif [ ${OPTION} == "integration_test" ]; then
    INTEGRATION_TEST=true
    SERVER_ADDRESS=$3
elif [ ${OPTION} == "install" ]; then
    INSTALL=true
elif [ ${OPTION} == "clean" ]; then
    rm -rf "build"
    rm -f ${ZEROMQ_DOWNLOAD} ${CPPZMQ_DOWNLOAD} ${MSGPACK_DOWNLOAD}
    rm -rf ${ZEROMQ_FOLDER} ${CPPZMQ_FOLDER} ${MSGPACK_DOWNLOAD}
    exit 0
else
    show_usage
    exit 1
fi

# install libzmq
if [ ! -d ${INSTALL_PATH}/zeromq ]; then
    maybe_download_zeromq

    tar -xzf ${ZEROMQ_DOWNLOAD} && rm ${ZEROMQ_DOWNLOAD} \
        && pushd ${ZEROMQ_FOLDER} \
        && ./configure -prefix=${INSTALL_PATH}/zeromq \
        && make -j ${nproc} && make install \
        && popd \
        && rm -r ${ZEROMQ_FOLDER} || exit 1
else
    echo -e "\nFound existing zeromq folder!\n"
fi

# install cppzmq
if [ ! -f ${INSTALL_PATH}/include/zmq.hpp ]; then
    maybe_download_cppzmq

    tar -xzf ${CPPZMQ_DOWNLOAD} && rm ${CPPZMQ_DOWNLOAD} \
        && mkdir -p ${INSTALL_PATH}/include \
        && cp ${CPPZMQ_FOLDER}/*.hpp ${INSTALL_PATH}/include \
        && rm -r ${CPPZMQ_FOLDER} || exit 1
else
    echo -e "\nFound existing cppzmq folder!\n"
fi

# install msgpack
if [ ! -f ${INSTALL_PATH}/include/msgpack.hpp ]; then
    maybe_download_msgpack

    tar -xzf ${MSGPACK_DOWNLOAD} && rm ${MSGPACK_DOWNLOAD} \
        && mkdir -p ${INSTALL_PATH}/include \
        && cp -r ${MSGPACK_FOLDER}/include ${INSTALL_PATH}/ \
        && rm -r ${MSGPACK_FOLDER} || exit 1
else
    echo -e "\nFound existing msgpack folder!\n"
fi

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

cmake -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL_PATH} \
    -DBUILD_TESTS=${TEST} \
    -DBUILD_INTEGRATION_TEST=${INTEGRATION_TEST} ..

make -j ${nproc}

if [ ${TEST} == true ]; then
    make test
fi

if [ ${INTEGRATION_TEST} == true ]; then
    integration_test/pysim_client ${SERVER_ADDRESS}
fi

if [ ${INSTALL} == true ]; then
    make install
fi
