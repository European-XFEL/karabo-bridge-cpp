FROM centos/python-36-centos7

RUN git clone https://github.com/European-XFEL/karabo-bridge-py.git \
  && pushd karabo-bridge-py \
  && pip install .

CMD ["/bin/bash"]
