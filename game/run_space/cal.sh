#!/bin/bash

echo "Card type win info:"
printf "%-18s: %5s %5s\r\n" "Type" "Total" "Win";
echo "===========================================";
all_total=0;
all_win=0;
for type in ROYAL_FLUSH STRAIGHT_FLUSH FOUR_OF_A_KIND FULL_HOUSE FLUSH STRAIGHT THREE_OF_A_KIND TWO_PAIR ONE_PAIR HIGH_CARD; do 
	total=$(grep -w "$type" ./log.txt | wc -l);
	win=$(grep -w "$type" ./log.txt | grep "1:"| wc -l);
	printf "%-18s: %5d %5d\r\n" "$type" $total $win;
	all_total=`expr $all_total + $total`;
	all_win=`expr $all_win + $win`;
done
echo "===========================================";
printf "%-18s: %5d %5d\r\n" "All Total" $all_total $all_win;

# 计算每个玩家的好牌情况
all_win=0;
echo ""
echo "User win info:"
printf "%-18s: %5s\r\n" "User ID" "Win";
echo "===========================================";
for user in 1001 1002 1003 1004 1005 1006 1007 1008; do
	win=$(cat log.txt | grep ^1: | grep "$user" | wc -l);
	all_win=`expr $all_win + $win`;
	printf "%-18s: %5d\r\n" $user $win; 
done
echo "===========================================";
printf "%-18s: %5d\r\n" "All Total" $all_win;
echo ""

echo "Cal msg info:"
echo -n "Get rounds:"
grep "=============Round" ./msg_log.log | wc -l;
echo "==========================================="
all_total=0;
all_win=0;
for type in ROYAL_FLUSH STRAIGHT_FLUSH FOUR_OF_A_KIND FULL_HOUSE FLUSH STRAIGHT THREE_OF_A_KIND TWO_PAIR ONE_PAIR HIGH_CARD; do 
	#printf "%-18s:" "$type"
	total=$(grep -w "$type" ./msg_log.log  | wc -l);
	printf "%-18s: %5d\r\n" "$type" $total;
	all_total=`expr $all_total + $total`;
done
echo "==========================================="
printf "%-18s: %5d %5d\r\n" "All Total" $all_total 0;

echo ""
echo "==========================================="
all_total=0;
all_win=0;
for type in CT_ROYAL_FLUSH CT_STRAIGHT_FLUSH CT_FOUR_OF_A_KIND CT_FULL_HOUSE CT_FLUSH CT_STRAIGHT CT_THREE_OF_A_KIND CT_TWO_PAIR CT_ONE_PAIR CT_HIGH_CARD; do 
	#printf "%-18s:" "$type"
	total=$(grep -w "$type" ./msg_log.log  | wc -l);
	printf "%-18s: %5d\r\n" "$type" $total;
	all_total=`expr $all_total + $total`;
done
echo "==========================================="
printf "%-18s: %5d %5d\r\n" "All Total" $all_total 0;
