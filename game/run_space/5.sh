#!/bin/bash

. ./evn.sh

# user 1
export "PLAYER1_IP"=127.0.0.1
export "PLAYER1_PORT"=6001
export "PLAYER1_ID"=1001

# user 2
export "PLAYER2_IP"=127.0.0.2
export "PLAYER2_PORT"=6002
export "PLAYER2_ID"=1002

# user 3
export "PLAYER3_IP"=127.0.0.3
export "PLAYER3_PORT"=6003
export "PLAYER3_ID"=1003

# user 4
export "PLAYER4_IP"=127.0.0.4
export "PLAYER4_PORT"=6004
export "PLAYER4_ID"=1004

# user 5
export "PLAYER5_IP"=127.0.0.5
export "PLAYER5_PORT"=6005
export "PLAYER5_ID"=1005

# Start Server
./gameserver -gip $ser_ip -seq replay -r $reg -d $delay -m $money -b $blind -t $jetton -h $hand 0</dev/null 1>/dev/null 2>/dev/null &

# Start all users
./game 127.0.0.1 6000 127.0.0.1 6001 1001  $1 &

#./game 127.0.0.1 6000 127.0.0.2 6002 1002 &
./check 127.0.0.1 6000 127.0.0.2 6002 1002 &

./check 127.0.0.1 6000 127.0.0.3 6003 1003 &

./check 127.0.0.1 6000 127.0.0.4 6004 1004 &

./check 127.0.0.1 6000 127.0.0.5 6005 1005 &
