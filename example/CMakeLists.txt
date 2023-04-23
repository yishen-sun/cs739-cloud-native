# cmake_minimum_required(VERSION 3.5)
# set(CMAKE_CXX_STANDARD 11)
# project(encoder LANGUAGES CXX)

# find_package(aws-lambda-runtime REQUIRED)
# find_package(AWSSDK COMPONENTS s3)
# find_package(ZLIB REQUIRED)

# add_executable(${PROJECT_NAME} "main.cpp")
# target_link_libraries(${PROJECT_NAME} PUBLIC
#                       AWS::aws-lambda-runtime
#                        ${AWSSDK_LINK_LIBRARIES})

# aws_lambda_package_target(${PROJECT_NAME})

cmake_minimum_required(VERSION 3.10)
project(s3_file_modifier)

set(CMAKE_CXX_STANDARD 17)

# Locate the AWS SDK for C++ package
find_package(AWSSDK REQUIRED COMPONENTS s3)
find_package(ZLIB REQUIRED)
# Include the AWS SDK for C++ headers
include_directories(${AWSSDK_INCLUDE_DIRS})

# Link against the AWS SDK for C++ libraries
link_libraries(${AWSSDK_LINK_LIBRARIES})

# Add the executable target and source files
add_executable(s3_file_modifier tmp.cpp)

# Link the executable target against the AWS SDK for C++ libraries
target_link_libraries(s3_file_modifier ${AWSSDK_LINK_LIBRARIES})
