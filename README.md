# cs739-cloud-native

### How to check aws credentials?
https://repost.aws/knowledge-center/s3-locate-credentials-error

### How to configure the AWS CLI?
https://docs.aws.amazon.com/cli/latest/userguide/cli-chap-configure.html#cli-configure-quickstart-config

aws configure list
aws configure

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