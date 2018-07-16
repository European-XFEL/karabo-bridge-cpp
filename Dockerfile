FROM zhujun98/maxwell

COPY . ./karabo-bridge-cpp

# build karabo-bridge-cpp
RUN pushd karabo-bridge-cpp \
  && if [ -d "build" ]; then rm -r build; fi \
  && mkdir build && pushd build \
  && cmake .. \
  && make -j${nproc}

CMD ["/bin/bash"]
