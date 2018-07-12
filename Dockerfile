FROM centos:7.0.1406

RUN yum install -y yum-plugin-ovl

RUN yum install -y gcc \
  && yum install -y gcc-c++ \
  && yum install -y cmake \
  && yum install -y make \
  && yum install -y nss \
  && yum install -y curl \
  && yum install -y git \
  && yum install -y wget \
  && yum install -y tar

RUN wget https://github.com/zeromq/libzmq/releases/download/v4.2.5/zeromq-4.2.5.tar.gz \
  && tar -xvzf zeromq-4.2.5.tar.gz \
  && pushd zeromq-4.2.5 \
  && ./configure -prefix=${HOME}/share/zeromq \
  && make -j${nproc} \
  && make install \
  && popd

RUN git clone https://github.com/European-XFEL/karabo-bridge-cpp.git \
  && pushd karabo-bridge-cpp \
  && wget https://github.com/zeromq/cppzmq/archive/v4.2.2.tar.gz \
  && tar -xvzf v4.2.2.tar.gz \
  && cp cppzmq-4.2.2/*.hpp external/cppzmq/ \
  && wget https://github.com/msgpack/msgpack-c/archive/cpp-2.1.5.tar.gz \
  && tar -xvzf cpp-2.1.5.tar.gz \
  && cp -r msgpack-c-cpp-2.1.5/include external/msgpack/ \
  && mkdir build && pushd build \
  && cmake .. \
  && make -j${nproc}

EXPOSE 1234 

CMD ["/bin/bash"]
