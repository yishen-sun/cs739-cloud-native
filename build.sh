mkdir build -p && cd build
cmake ../src
cmake --build .
cd ..

mkdir -p release
cp build/client_cli release/client_cli
cp build/dynamo_server release/dynamo_server
cp build/consistent_hashing_ring_test release/consistent_hashing_ring_test
cp build/state_machine_test release/state_machine_test
cp build/admin_cli release/admin_cli
cp build/multiple_client_test release/multiple_client_test
cp build/admin_auto_start release/admin_auto_start