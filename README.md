# cs739-cloud-native

### Cmd
```
sh build.sh

# Initialize five node servers
./release/dynamo_server 0.0.0.0:50001 3 0.0.0.0:50001 50001_config.txt
./release/dynamo_server 0.0.0.0:60001 3 0.0.0.0:60001 60001_config.txt
./release/dynamo_server 0.0.0.0:70001 3 0.0.0.0:70001 70001_config.txt
./release/dynamo_server 0.0.0.0:80001 3 0.0.0.0:80001 80001_config.txt
./release/dynamo_server 0.0.0.0:90001 3 0.0.0.0:90001 90001_config.txt

# Use admin to trigger node join the network
./release/admin_cli server_config_for_admin admin_sm.txt
JoinNetwork 0.0.0.0:50001
JoinNetwork 0.0.0.0:60001
JoinNetwork 0.0.0.0:70001
JoinNetwork 0.0.0.0:80001
JoinNetwork 0.0.0.0:90001

# Initialize client side
./release/client_cli server_config_for_admin.txt 0.0.0.0:50001
```

```
# Multiple node configuration, run each line in each machine

./dynamo_server [node_1 address]:50001 3 [node_1 address]:50001 config.txt

./dynamo_server [node_2 address]:50001 3 [node_2 address]:50001 config.txt

./dynamo_server [node_3 address]:50001 3 [node_3 address]:50001 config.txt

./dynamo_server [node_4 address]:50001 3 [node_4 address] config.txt

./dynamo_server [node_5 address]:50001 3 [node_5 address]:50001 config.txt

# Use admin to trigger nodes.
JoinNetwork [node_1 address]:50001
JoinNetwork [node_2 address]:50001
JoinNetwork [node_3 address]:50001
JoinNetwork [node_4 address]:50001
JoinNetwork [node_5 address]:50001
```