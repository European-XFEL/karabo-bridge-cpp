FROM centos:7.0.1406

# avoid checksum error
RUN yum install -y yum-plugin-ovl

# gcc, g++ 4.8.5 and cmake 2.8.12
RUN yum install -y gcc gcc-c++ cmake make && \
    yum install -y nss curl git wget tar

ENV LD_LIBRARY_PATH="/root/miniconda/lib"
ENV PATH="/root/miniconda/bin:$PATH"
RUN wget "http://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh" -O miniconda.sh && \
    mkdir ${HOME}/.conda && \
    bash miniconda.sh -b -p ${HOME}/miniconda && \
    conda config --set always_yes yes --set changeps1 no && \
    conda update -q conda && \
    # install dependencies
    conda install -c anaconda cmake && \
    conda install -c omgarcia gcc-6 && \
    conda install -c conda-forge cppzmq msgpack-c

COPY . ./karabo-bridge-cpp

# build karabo-bridge-cpp
RUN cd karabo-bridge-cpp && if [ -d build ]; then rm -r build; fi && \
    mkdir build && cd build && \
    cmake -DBUILD_TESTS=ON -DBUILD_INTEGRATION_TEST=ON -DBUILD_EXAMPLES=ON ../ && \
    make && make test

CMD ["/bin/bash"]
