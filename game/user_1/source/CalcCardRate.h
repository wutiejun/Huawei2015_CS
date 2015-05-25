#pragma once

#include "tx_poker.h"

#define CardMask_RESET(mask) \
    do { (mask).cards_n = 0; } while (0)

#define CardMask_ANY_SET(mask1, mask2)                          \
    (( (mask1).cards_n & (mask2).cards_n) != 0 )                 

#define LongLong_OP(result, op1, op2, operation) \
    do { result = (op1) operation (op2); } while (0)

#define CardMask_OP(result, op1, op2, OP) \
    LongLong_OP((result).cards_n, (op1).cards_n, (op2).cards_n, OP);

#define CardMask_OR(result, op1, op2) \
    CardMask_OP(result, op1, op2, |)

#define CardMask_AND(result, op1, op2) \
    CardMask_OP(result, op1, op2, &)

#define CardMask_XOR(result, op1, op2) \
    CardMask_OP(result, op1, op2, ^)

#define CardMask_SET(mask, index)       \
    do {                                            \
    CardMask _t1 = POKER_MASK(index);           \
    CardMask_OR((mask), (mask), _t1);             \
    } while (0)


extern uint8 nBitsTable[N_RANKMASKS];
extern uint32 topFiveCardsTable[N_RANKMASKS];
extern uint32 topFiveBitTable[N_RANKMASKS];
extern uint8 topCardTable[N_RANKMASKS];
extern uint32 topBitTable[N_RANKMASKS];
extern uint32 topTwoBitsTable[N_RANKMASKS];
extern uint32 topFiveBitsTable[N_RANKMASKS];
extern uint8 straightTable[N_RANKMASKS];
extern uint8 nBitsAndStrTable[N_RANKMASKS];

extern CardMask cardMasksTable[POKER_N_CARDS];

struct CardStatCount {
    unsigned int win_count;
    unsigned int fail_count;
    unsigned int tie_count;
    unsigned int total_count;
};

CardStatCount CalcCardsRate(int n_cards, CardMask dead_cards, CardMask p0, CardMask p1);
int GetCardsType( CardMask cards, int n_cards );
CardStatCount CalcPlayerStatCount(CardMask player_cards, CardMask dead_cards, CardMask common_cards);
CardStatCount CalcPlayerStatCount_E(CardMask player_cards, CardMask dead_cards, CardMask common_cards);
