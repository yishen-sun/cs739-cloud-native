#!/bin/bash

# Define a list of remote servers
SERVERS=("yucht@c220g2-011131.wisc.cloudlab.us" "yucht@c220g2-011129.wisc.cloudlab.us" "yucht@c220g2-011109.wisc.cloudlab.us"
"yucht@c220g2-011124.wisc.cloudlab.us"
"yucht@c220g2-011106.wisc.cloudlab.us")
# "yucht@c220g2-011114.wisc.cloudlab.us"
# "yucht@c220g2-011105.wisc.cloudlab.us"
# "yucht@c220g2-011111.wisc.cloudlab.us"
# "yucht@c220g2-011101.wisc.cloudlab.us"
# "yucht@c220g2-011116.wisc.cloudlab.us")

# Loop through the servers and run the commands
for (( i=0; i<${#SERVERS[@]}; i++ ))
do
  SERVER="${SERVERS[i]}"
  HOST=$(echo $SERVER | cut -d "@" -f2)
  COMMAND="kill \$(ps aux | grep c220 | awk '{print \$2}')"
  COMMAND2="rm -f c220*"
  echo "Running command on $SERVER: ${COMMAND}"
  ssh $SERVER $COMMAND
  ssh $SERVER $COMMAND2
done
wait