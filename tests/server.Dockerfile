FROM centos/python-36-centos7

RUN git clone https://github.com/European-XFEL/karabo-bridge-py.git \
  && pushd karabo-bridge-py \
  && pip install .

CMD ["karabo-bridge-server-sim", "1234", "-n", "2"]
