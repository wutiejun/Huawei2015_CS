#!/bin/bash

echo "Cal log info:"
echo "==========================================="
for type in ROYAL_FLUSH STRAIGHT_FLUSH FOUR_OF_A_KIND FULL_HOUSE FLUSH STRAIGHT THREE_OF_A_KIND TWO_PAIR ONE_PAIR HIGH_CARD; do 
	printf "%-18s:" "$type"
	grep -w "$type" ./log.txt | wc -l;
done

echo "Cal msg info:"
echo -n "Get rounds:"
grep "msg_reader.cpp:802" ./msg_log.log | wc -l;
echo "==========================================="
for type in ROYAL_FLUSH STRAIGHT_FLUSH FOUR_OF_A_KIND FULL_HOUSE FLUSH STRAIGHT THREE_OF_A_KIND TWO_PAIR ONE_PAIR HIGH_CARD; do 
	printf "%-18s:" "$type"
	grep -w "$type" ./msg_log.log  | wc -l;
done
echo "==========================================="
for type in CT_ROYAL_FLUSH CT_STRAIGHT_FLUSH CT_FOUR_OF_A_KIND CT_FULL_HOUSE CT_FLUSH CT_STRAIGHT CT_THREE_OF_A_KIND CT_TWO_PAIR CT_ONE_PAIR CT_HIGH_CARD; do 
	printf "%-18s:" "$type"
	grep -w "$type" ./msg_log.log | wc -l;
done
echo "==========================================="