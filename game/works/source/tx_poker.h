#pragma once

typedef unsigned char  uint8;
typedef signed char   int8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long long uint64;

typedef union {

  uint64  cards_n;

  struct {
    uint32 spades  :13;   //ºÚÌÒ
    uint32         : 3;
    uint32 clubs   :13;   //Ã·»¨
    uint32         : 3;
    uint32 diamonds:13;   //·½¿é
    uint32         : 3;
    uint32 hearts  :13;   //ºìÌÒ
    uint32         : 3;
  } cards;

} CardMask;

#define POKER_N_CARDS      52
#define POKER_MASK(index)  (cardMasksTable[index])

#define CardMask_SPADES(cm)   ((cm).cards.spades)
#define CardMask_CLUBS(cm)    ((cm).cards.clubs)
#define CardMask_DIAMONDS(cm) ((cm).cards.diamonds)
#define CardMask_HEARTS(cm)   ((cm).cards.hearts)

#define HandType_NOPAIR    0
#define HandType_ONEPAIR   1
#define HandType_TWOPAIR   2
#define HandType_TREEKIND     3
#define HandType_STRAIGHT  4
#define HandType_FLUSH     5
#define HandType_FULLHOUSE 6
#define HandType_FOURKIND     7
#define HandType_STFLUSH   8

#define HandVal_HANDTYPE_SHIFT    24
#define HandVal_HANDTYPE_MASK     0x0F000000
#define HandVal_CARDS_SHIFT       0
#define HandVal_CARDS_MASK        0x000FFFFF
#define HandVal_TOP_CARD_SHIFT    16
#define HandVal_TOP_CARD_MASK     0x000F0000
#define HandVal_SECOND_CARD_SHIFT 12
#define HandVal_SECOND_CARD_MASK  0x0000F000
#define HandVal_THIRD_CARD_SHIFT  8
#define HandVal_THIRD_CARD_MASK   0x00000F00
#define HandVal_FOURTH_CARD_SHIFT 4
#define HandVal_FOURTH_CARD_MASK  0x000000F0
#define HandVal_FIFTH_CARD_SHIFT  0
#define HandVal_FIFTH_CARD_MASK   0x0000000F
#define HandVal_CARD_WIDTH        4
#define HandVal_CARD_MASK         0x0F

#define HandVal_HANDTYPE_VALUE(ht)   ((ht) << HandVal_HANDTYPE_SHIFT)
#define HandVal_TOP_CARD_VALUE(c)    ((c) << HandVal_TOP_CARD_SHIFT)
#define HandVal_SECOND_CARD_VALUE(c) ((c) << HandVal_SECOND_CARD_SHIFT)
#define HandVal_THIRD_CARD_VALUE(c)  ((c) << HandVal_THIRD_CARD_SHIFT)
#define HandVal_FOURTH_CARD_VALUE(c) ((c) << HandVal_FOURTH_CARD_SHIFT)
#define HandVal_FIFTH_CARD_VALUE(c)  ((c) << HandVal_FIFTH_CARD_SHIFT)

#define Rank_2      0
#define Rank_3      1
#define Rank_4      2
#define Rank_5      3
#define Rank_6      4
#define Rank_7      5
#define Rank_8      6
#define Rank_9      7
#define Rank_TEN    8
#define Rank_JACK   9
#define Rank_QUEEN  10
#define Rank_KING   11
#define Rank_ACE    12
#define Rank_COUNT  13
#define Rank_FIRST  Rank_2
#define Rank_LAST   Rank_ACE
#define N_RANKMASKS (1 << Rank_COUNT)

#define RANK(index)  ((index) % Rank_COUNT)
#define SUIT(index)  ((index) / Rank_COUNT)


#define Suit_HEARTS   0
#define Suit_DIAMONDS 1
#define Suit_CLUBS    2
#define Suit_SPADES   3
#define Suit_FIRST    Suit_HEARTS
#define Suit_LAST     Suit_SPADES
#define Suit_COUNT    4

#define MAKE_CARD(rank, suit) ((suit * Rank_COUNT) + rank)