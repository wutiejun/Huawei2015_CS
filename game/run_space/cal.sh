#!/bin/bash

function CalTypeWinRation()
{
	printf "\r\nCard type win info:\r\n"
	printf "%-18s: %5s %5s\r\n" "Type" "Total" "Win";
	printf "===========================================\r\n";
	all_total=0;
	all_win=0;
	for type in ROYAL_FLUSH STRAIGHT_FLUSH FOUR_OF_A_KIND FULL_HOUSE FLUSH STRAIGHT THREE_OF_A_KIND TWO_PAIR ONE_PAIR HIGH_CARD; do 
		total=$(grep -w "$type" ./log.txt | wc -l);
		win=$(grep -w "$type" ./log.txt | grep "1:"| wc -l);
		printf "%-18s: %5d %5d\r\n" "$type" $total $win;
		all_total=`expr $all_total + $total`;
		all_win=`expr $all_win + $win`;
	done
	printf "===========================================\r\n";
	printf "%-18s: %5d %5d\r\n" "All Total" $all_total $all_win;
}

function CalTypeDevRation()
{
	# 生成临时文件
	rm -f "./tempfile.txt";
	for type in ROYAL_FLUSH STRAIGHT_FLUSH FOUR_OF_A_KIND FULL_HOUSE FLUSH STRAIGHT THREE_OF_A_KIND TWO_PAIR ONE_PAIR HIGH_CARD; do 
		grep -w "$type" ./log.txt >> "./tempfile.txt";		
	done;
	
	rm -f ./temp_same_color.txt
	cat "./tempfile.txt" | grep -v CLUBS | grep -v DIAMONDS | grep -v HEARTS >> ./temp_same_color.txt
	cat "./tempfile.txt" | grep -v CLUBS | grep -v DIAMONDS | grep -v SPADES >> ./temp_same_color.txt
	cat "./tempfile.txt" | grep -v CLUBS | grep -v SPADES | grep -v HEARTS >> ./temp_same_color.txt
	cat "./tempfile.txt" | grep -v SPADES | grep -v DIAMONDS | grep -v HEARTS >> ./temp_same_color.txt
	
	# 计算同花出现的次数
	all_total=0;
	all_win=0;
	total=$(cat "./tempfile.txt" | grep -v CLUBS | grep -v DIAMONDS | grep -v HEARTS | wc -l);
	win=$(cat "./tempfile.txt" | grep -v CLUBS | grep -v DIAMONDS | grep -v HEARTS | grep "1:" |wc -l);
	printf "total %d win %d\r\n" $total $win
	all_total=`expr $all_total + $total`;
	all_win=`expr $all_win + $win`;
	#
	total=$(cat "./tempfile.txt" | grep -v CLUBS | grep -v DIAMONDS | grep -v SPADES | wc -l);
	win=$(cat "./tempfile.txt" | grep -v CLUBS | grep -v DIAMONDS | grep -v SPADES | grep "1:" |wc -l);
	printf "total %d win %d\r\n" $total $win
	all_total=`expr $all_total + $total`;
	all_win=`expr $all_win + $win`;
	#
	total=$(cat "./tempfile.txt" | grep -v CLUBS | grep -v SPADES | grep -v HEARTS | wc -l);
	win=$(cat "./tempfile.txt" | grep -v CLUBS | grep -v SPADES | grep -v HEARTS | grep "1:" |wc -l);
	printf "total %d win %d\r\n" $total $win
	all_total=`expr $all_total + $total`;
	all_win=`expr $all_win + $win`;
	#
	total=$(cat "./tempfile.txt" | grep -v SPADES | grep -v DIAMONDS | grep -v HEARTS | wc -l);
	win=$(cat "./tempfile.txt" | grep -v SPADES | grep -v DIAMONDS | grep -v HEARTS | grep "1:" |wc -l);
	printf "total %d win %d\r\n" $total $win
	all_total=`expr $all_total + $total`;
	all_win=`expr $all_win + $win`;
	printf "Same color %6d %6d\r\n" $all_total $all_win;	
}

function CalCardShowAndWinRation()
{
	printf "\r\nCard point win info:\r\n"
	printf "Point  Show   Win\r\n"
	all_total=0;
	all_win=0;
	printf "===========================================\r\n";
	for card in 2 3 4 5 6 7 8 9 10 J Q K A; do
		#echo $card 		
		show_times=$(cat log.txt | grep "^[1-8]:" | cut -c3- | grep -w "$card" | wc -l);
		win_times=$(cat log.txt | grep "1:" | grep -w "$card" | wc -l);
		printf "%-5s %5d %5d\r\n" $card $show_times $win_times
		all_total=`expr $all_total + $show_times`;
		all_win=`expr $all_win + $win_times`;
	done
	printf "===========================================\r\n";
	printf "Total:%5d %5d\r\n" $all_total $all_win;
}

function CalPlayerWinRation()
{
	printf "\r\nPlayer win info:\r\n"
	printf "Player   Win\r\n"
	all_win=0;
	printf "===========================================\r\n";
	for player in 1001 1002 1003 1004 1005 1006 1007 1008; do
		#echo $card 	
		win_times=$(cat log.txt | grep "^[1-8]:" | grep -w "1:" | grep -w "$player"  | wc -l);
		if [ "$win_times" != "0" ]; then
			printf "%-5s: %5d\r\n" $player $win_times
		fi
		all_win=`expr $all_win + $win_times`;
	done
	printf "===========================================\r\n";
	printf "Total: %5d\r\n" $all_win;
}

#CalTypeWinRation

#CalCardShowAndWinRation

#CalPlayerWinRation

#CalTypeDevRation

# 看最后谁的金币最多
cat ./log.txt | grep " blind: " | tail

