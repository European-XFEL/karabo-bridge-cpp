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

COPY . ./karabo-bridge-cpp

# build karabo-bridge-cpp
RUN pushd karabo-bridge-cpp && ./autogen.sh clean && ./autogen.sh test 

CMD ["/bin/bash"]
