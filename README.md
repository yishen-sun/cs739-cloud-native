# cs739-cloud-native

Dynamo is a highly available key-value storage designed by Amazon. The main focus is to give clients an "always-on" experience. Under the CAP theorem, the design focuses on availability and partition at the cost of consistency. Compared to Raft, Dynamo doesn't have a leader server to keep request order and maintain strong consistency, while every peer can serve as a "coordinator" if they take responsibility for those keys. Five major techniques are introduced to solve potential problems. First, the consistent hashing technique is used to distribute keys among different servers. The key is hashed and assigned to specific groups of servers to handle it. Second, vector clocks with reconciliation during reads are introduced to improve performance. It's possible that the system experiences temporary failures or other problems, resulting in conflicting records. To improve performance, the system allows users to write data and delay resolving the conflict records. Third, sloppily quorum and hinted handoff are introduced to handle temporary failures. Given that a key is assigned to three servers (A, B, C), server D or another server may take responsibility for the data if one of them is temporarily down to maintain high availability. Fourth, a gossip-based membership protocol is introduced to manage membership and detect node failures. Lastly, anti-entropy using Merkle trees is introduced to solve permanent failures. The hash trees are maintained on each server, and they can verify the correctness of records by passing the hashing information instead of all the data they have, which improves the cost of comparing that data.

We implemented the first four techniques in this project. The system is composed of client and server parts. The client library enables clients to read or write data into the system, while the server library enables users to run peer servers to handle requests from clients, administrators, and other servers. The administrator library allows administrators to notify the existing network that a new physical node will join the network or that a failed node has restarted and will resume its responsibility for partitioning data. Each library communicates with each others using gRPC. The whole project is implemented in C++, and we only store the key-value data in a standard unordered map library for the state machine.

For more details, please check out our [report](./CS_739_Dynamo_report.pdf).

### How to check aws credentials?

https://repost.aws/knowledge-center/s3-locate-credentials-error

### How to configure the AWS CLI?

https://docs.aws.amazon.com/cli/latest/userguide/cli-chap-configure.html#cli-configure-quickstart-config

### aws configure list

https://docs.aws.amazon.com/cli/latest/userguide/sso-configure-profile-token.html#sso-configure-profile-token-auto-sso
https://docs.aws.amazon.com/sdkref/latest/guide/access-sso.html
https://github.com/awsdocs/aws-doc-sdk-examples/tree/main/cpp/example_code/s3

### Cmd

```
sh build.sh
./release/gossip_node_test 0.0.0.0:50001 3 0.0.0.0:50001 50001_config.txt
./release/gossip_node_test 0.0.0.0:60001 3 0.0.0.0:60001 60001_config.txt
./release/gossip_node_test 0.0.0.0:70001 3 0.0.0.0:70001 70001_config.txt
./release/gossip_node_test 0.0.0.0:80001 3 0.0.0.0:80001 80001_config.txt
./release/gossip_node_test 0.0.0.0:90001 3 0.0.0.0:90001 90001_config.txt

JoinNetwork 0.0.0.0:50001
JoinNetwork 0.0.0.0:60001
JoinNetwork 0.0.0.0:70001
JoinNetwork 0.0.0.0:80001
JoinNetwork 0.0.0.0:90001
```

```
./gossip_node_test c220g5-111207.wisc.cloudlab.us:50001 3 c220g5-111207.wisc.cloudlab.us:50001 config.txt
./gossip_node_test c220g5-111232.wisc.cloudlab.us:50001 3 c220g5-111232.wisc.cloudlab.us:50001 config.txt
./gossip_node_test c220g5-111202.wisc.cloudlab.us:50001 3 c220g5-111202.wisc.cloudlab.us:50001 config.txt
./gossip_node_test c220g5-111216.wisc.cloudlab.us:50001 3 c220g5-111216.wisc.cloudlab.us:50001 config.txt
./gossip_node_test c220g5-111220.wisc.cloudlab.us:50001 3 c220g5-111220.wisc.cloudlab.us:50001 config.txt

JoinNetwork c220g5-111207.wisc.cloudlab.us:50001
JoinNetwork c220g5-111232.wisc.cloudlab.us:50001
JoinNetwork c220g5-111202.wisc.cloudlab.us:50001
JoinNetwork c220g5-111216.wisc.cloudlab.us:50001
JoinNetwork c220g5-111220.wisc.cloudlab.us:50001
```
