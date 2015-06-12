#!/bin/bash

# user 1
export "PLAYER1_IP"=127.0.0.1
export "PLAYER1_PORT"=6001
export "PLAYER1_ID"=1001

# user 2
export "PLAYER2_IP"=127.0.0.2
export "PLAYER2_PORT"=6002
export "PLAYER2_ID"=1002

# Start Server
./gameserver -gip 127.0.0.1 -seq replay -r 30 -d 1 -m 10000 -b 50 -t 2000 -h 500 0</dev/null 1>/dev/null 2>/dev/null &

# Start all users
if [ "$1" == "" ]; then
	./game 127.0.0.1 6000 127.0.0.1 6001 1001  &
fi

if [ "$1" == "check" ]; then
	./check 127.0.0.1 6000 127.0.0.1 6001 1001  &
fi

./check 127.0.0.1 6000 127.0.0.2 6002 1002 &
