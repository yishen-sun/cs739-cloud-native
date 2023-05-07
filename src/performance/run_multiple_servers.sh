#!/bin/bash
# Define a list of remote servers
SERVERS=("yucht@c220g1-031103.wisc.cloudlab.us" "yucht@c220g1-031130.wisc.cloudlab.us" "yucht@c220g1-031125.wisc.cloudlab.us")

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