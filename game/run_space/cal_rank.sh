#!/bin/bash

function CalPlayerRank()
{
	rm -f ./rank.temp
	for player in 1001 1002 1003 1004 1005 1006 1007 1008; do
		cat ./log.txt | grep " blind: $player" | tail -n 1 | cut -d" " -f3,4,5 >> ./rank.temp
	done
	
	rm -f ./rank2.temp
	while read pid jetton money; do
		total=`expr $jetton + $money`;
		printf "%6d %s %5d %5d\r\n" $total $pid $jetton $money >> rank2.temp
	done < ./rank.temp
	
	printf "========= RANK ===========\r\n"
	printf "Total  PID  Jetton Money \r\n"
	printf "==========================\r\n"
	cat rank2.temp | sort -gr -k1,7
	printf "==========================\r\n"
}

CalPlayerRank
