# install cmake
sudo apt update -y
sudo apt install -y cmake

# install libs
sudo apt install -y libcurl4-openssl-dev

# install awscli
# https://docs.aws.amazon.com/cli/latest/userguide/cli-chap-welcome.html
sudo apt install awscli -y

# Download and compile the C++ Lambda Runtime
# https://aws.amazon.com/blogs/compute/introducing-the-c-lambda-runtime/
# This builds and installs the runtime as a static library under the directory ~/out.
git clone https://github.com/awslabs/aws-lambda-cpp.git
cd aws-lambda-cpp
mkdir build -p
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF \
   -DCMAKE_INSTALL_PREFIX=~/out
make && make install

cd ../..

# Download and compile the C++ SDK
# https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/setup-linux.html
sudo apt install -y libcurl4-openssl-dev libssl-dev uuid-dev zlib1g-dev libpulse-dev

git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp
cd aws-sdk-cpp
mkdir build -p
cd build
cmake .. -DBUILD_ONLY=s3 \
 -DBUILD_SHARED_LIBS=OFF \
 -DENABLE_UNITY_BUILD=ON \
 -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_INSTALL_PREFIX=~/out
make && make install
