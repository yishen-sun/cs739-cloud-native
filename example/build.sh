mkdir build -p
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=~/out
make
make aws-lambda-package-encoder