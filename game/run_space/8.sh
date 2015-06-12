#!/bin/bash

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

# user 6
export "PLAYER6_IP"=127.0.0.6
export "PLAYER6_PORT"=6006
export "PLAYER6_ID"=1006

# user 6
export "PLAYER7_IP"=127.0.0.7
export "PLAYER7_PORT"=6007
export "PLAYER7_ID"=1007

# user 7
export "PLAYER8_IP"=127.0.0.8
export "PLAYER8_PORT"=6008
export "PLAYER8_ID"=1008

# Start Server
./gameserver -gip 127.0.0.1 -seq replay -r 30 -d 1 -m 10000 -b 50 -t 2000 -h 500 0</dev/null 1>/dev/null 2>/dev/null &

# Start all users
if [ "$1" == "" ]; then
	./game 127.0.0.1 6000 127.0.0.1 6001 1001  &
fi

if [ "$1" == "check" ]; then
	./check 127.0.0.1 6000 127.0.0.1 6001 1001  &
fi

#./game 127.0.0.1 6000 127.0.0.2 6002 1002 &
./check 127.0.0.1 6000 127.0.0.2 6002 1002 &

./check 127.0.0.1 6000 127.0.0.3 6003 1003 &

./check 127.0.0.1 6000 127.0.0.4 6004 1004 &

./check 127.0.0.1 6000 127.0.0.5 6005 1005 &

./check 127.0.0.1 6000 127.0.0.6 6006 1006 &

./check 127.0.0.1 6000 127.0.0.7 6007 1007 &

./check 127.0.0.1 6000 127.0.0.8 6008 1008 &
