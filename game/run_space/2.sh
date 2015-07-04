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

<<<<<<< HEAD
run_index=1


=======
>>>>>>> parent of 7a65cf3... 增加脚本，计算排名。
# Start Server
./gameserver -gip $ser_ip -seq replay -r $reg -d $delay -m $money -b $blind -t $jetton -h $hand 0</dev/null 1>/dev/null 2>/dev/null &

# Start all users
<<<<<<< HEAD
./game 127.0.0.1 6000 127.0.0.1 6001 1001 & 

./check 127.0.0.1 6000 127.0.0.2 6002 1002 & 

=======
./game 127.0.0.1 6000 127.0.0.1 6001 1001 $1 &
>>>>>>> parent of 7a65cf3... 增加脚本，计算排名。

./check 127.0.0.1 6000 127.0.0.2 6002 1002 &
