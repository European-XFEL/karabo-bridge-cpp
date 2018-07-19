FROM centos:7.0.1406

# avoid checksum error
RUN yum install -y yum-plugin-ovl

# gcc, g++ 4.8.5 and cmake 2.8.12
RUN yum install -y gcc && \
    yum install -y gcc-c++ && \
    yum install -y cmake && \
    yum install -y make && \
    yum install -y nss && \
    yum install -y curl && \
    yum install -y git && \
    yum install -y wget && \
    yum install -y tar && \
    yum clean all

# install libzmq
RUN wget https://github.com/zeromq/libzmq/releases/download/v4.2.5/zeromq-4.2.5.tar.gz \
  && tar -xzf zeromq-4.2.5.tar.gz && rm zeromq-4.2.5.tar.gz \
  && pushd zeromq-4.2.5 \
  && ./configure -prefix=${HOME}/share/zeromq \
  && make \
  && make install \
  && popd \
  && rm -r zeromq-4.2.5

# install cppzmq
RUN wget https://github.com/zeromq/cppzmq/archive/v4.2.2.tar.gz \
  && tar -xzf v4.2.2.tar.gz \
  && rm v4.2.2.tar.gz \
  && mkdir -p ${HOME}/share/cppzmq/include \
  && cp cppzmq-4.2.2/*.hpp ${HOME}/share/cppzmq/include \
  && rm -r cppzmq-4.2.2

# install msgpack
RUN wget https://github.com/msgpack/msgpack-c/archive/cpp-2.1.5.tar.gz \
  && tar -xzf cpp-2.1.5.tar.gz \
  && rm cpp-2.1.5.tar.gz \
  && mkdir -p ${HOME}/share/msgpack \
  && cp -r msgpack-c-cpp-2.1.5/include ${HOME}/share/msgpack/ \
  && rm -r msgpack-c-cpp-2.1.5

COPY . ./karabo-bridge-cpp

# build karabo-bridge-cpp
RUN pushd karabo-bridge-cpp \
  && if [ -d "build" ]; then rm -r build; fi \
  && mkdir build && pushd build \
  && cmake .. \
  && make -j${nproc}

CMD ["/bin/bash"]
