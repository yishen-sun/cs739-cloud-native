export MY_INSTALL_DIR=$HOME/.local
mkdir -p $MY_INSTALL_DIR
export PATH="$MY_INSTALL_DIR/bin:$PATH"

sudo apt update -y

sudo apt install -y cmake

sudo apt install -y build-essential autoconf libtool pkg-config libsystemd-dev 

sudo apt install -y libprotobuf-dev protobuf-compiler
export Protobuf_DIR=/usr/lib/x86_64-linux-gnu/cmake/protobuf


git clone --recurse-submodules -b v1.52.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc

cd grpc
mkdir -p cmake/build && cd cmake/build
cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR \
      ../..
make -j 4
make install
cd ../..