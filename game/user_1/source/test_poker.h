#pragma once

#define Poker_maskString(m)      \
    GenericPoker_maskString(&StdDeck, ((void *) &(m)))

#define STRING_CARDS 100

typedef int     Poker_cardToStringFn(int cardIndex, char *outString);
typedef int     Poker_maskToCardsFn(void *cardMask, int cardIndices[]);
typedef int     Poker_stringToCardFn(char *inString, int *index);
typedef int     Poker_numCardsFn(void *cardMask);

typedef struct {
    int                  nCards;
    Poker_cardToStringFn *cardToString;
    Poker_stringToCardFn *stringToCard;
    Poker_maskToCardsFn  *maskToCards;
    Poker_numCardsFn     *numCards;
} Deck;

extern Deck StdDeck;

extern char * GenericPoker_maskString(Deck *deck, void *cardMask);

#define CardMask_CARD_IS_SET(mask, index)                       \
    (( (mask).cards_n & (POKER_MASK(index).cards_n)) != 0 )                 

