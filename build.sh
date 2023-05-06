if [ "$1" = "basic" ]; then
    echo "The second argument is basic."
    mkdir -p build
    cd build
    cmake ../src
    cmake --build .
    cd ..
elif [ "$1" = "s3" ]; then
    echo "The second argument is s3."
    mkdir -p build
    cd build
    cmake ../src -DUSE_S3_ADMIN=ON -DCMAKE_BUILD_TYPE=Release
    cmake --build .
    cd ..
else
    echo "You should input s3 or basic as the second argument."
fi

mkdir -p release
cp build/client_cli release/client_cli
cp build/gossip_node_test release/gossip_node_test
cp build/consistent_hashing_ring_test release/consistent_hashing_ring_test
cp build/state_machine_test release/state_machine_test
cp build/admin_cli release/admin_cli