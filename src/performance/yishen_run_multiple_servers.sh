#!/bin/bash
# Define a list of remote servers
# c220g5-111207.wisc.cloudlab.us:50001
# c220g5-111232.wisc.cloudlab.us:50001
# c220g5-111202.wisc.cloudlab.us:50001
# c220g5-111216.wisc.cloudlab.us:50001
# c220g5-111220.wisc.cloudlab.us:50001
SERVERS=("yishen@c220g5-111207.wisc.cloudlab.us" "yishen@c220g5-111232.wisc.cloudlab.us" "yishen@c220g5-111202.wisc.cloudlab.us" "yishen@c220g5-111216.wisc.cloudlab.us" "yishen@c220g5-111220.wisc.cloudlab.us")

# Loop through the servers and run the commands
for (( i=0; i<${#SERVERS[@]}; i++ ))
do
  SERVER="${SERVERS[i]}"
  HOST=$(echo $SERVER | cut -d "@" -f2)
  COMMAND=("cd tmp && ./gossip_node_test $HOST:50001 3 $HOST:50001 one_server_config.txt")
  echo "Running command on $SERVER: ${COMMAND[@]}"
  ssh $SERVER "${COMMAND[@]}" &
done
wait