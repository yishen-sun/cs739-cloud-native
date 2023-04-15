# install cmake
sudo apt update -y
sudo apt install -y cmake

# install libs
sudo apt install -y libcurl4-openssl-dev

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

# install awscli
# https://docs.aws.amazon.com/cli/latest/userguide/cli-chap-welcome.html
sudo apt install awscli -y

# How to check aws credentials?
# https://repost.aws/knowledge-center/s3-locate-credentials-error

# How to configure the AWS CLI?
# https://docs.aws.amazon.com/cli/latest/userguide/cli-chap-configure.html#cli-configure-quickstart-config
# aws configure list
# aws configure

# How to create a role? 
# aws iam create-role --role-name lambda-cpp-demo --assume-role-policy-document file://trust-policy.json

# aws lambda create-function \
# --function-name hello-world \
# --role arn:aws:iam::506805517360:role/lambda-cpp-demo \
# --runtime provided \
# --timeout 15 \
# --memory-size 128 \
# --handler helloworld \
# --zip-file fileb://helloworld.zip


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

# add S3 access to the role
# aws iam attach-role-policy \
# --role-name lambda-cpp-demo \
# --policy-arn arn:aws:iam::aws:policy/AmazonS3ReadOnlyAccess


# aws lambda create-function \
# --function-name encode-file \
# --role arn:aws:iam::506805517360:role/lambda-cpp-demo \
# --runtime provided \
# --timeout 15 \
# --memory-size 128 \
# --handler encoder \
# --zip-file fileb://encoder.zip

# aws lambda invoke --function-name encode-file --payload '{"s3bucket": "bucketencoder", "s3key":"test.txt" }' base64_image.txt