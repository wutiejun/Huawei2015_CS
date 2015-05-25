#include <stdio.h>
#include "test_poker.h"
#include "tx_poker.h"
#include <locale>
#include "CalcCardRate.h"
#include <string.h>
#include <stdlib.h>

int gNCommon, gNDead;
CardMask gDeadCards, gCommonCards, gTestPlayerCards[2];

char Poker_rankChars[] = "23456789TJQKA";
char Poker_suitChars[] = "hdcs";


int 
Poker_stringToCard(char *inString, int *cardIndex) {
    char *p;
    int rank, suit;

    p = inString;
    for (rank=Rank_FIRST; rank <= Rank_LAST; rank++) 
        if (Poker_rankChars[rank] == toupper(*p))
            break;
    if (rank > Rank_LAST)
        goto noMatch;
    ++p;
    for (suit=Suit_FIRST; suit <= Suit_LAST; suit++) 
        if (Poker_suitChars[suit] == tolower(*p))
            break;
    if (suit > Suit_LAST)
        goto noMatch;
    *cardIndex = MAKE_CARD(rank, suit);
    return 2;

noMatch:
    /* Didn't match anything, return failure */
    return 0;
}

int 
Poker_cardToString(int cardIndex, char *outString) {
    *outString++ = Poker_rankChars[RANK(cardIndex)];
    *outString++ = Poker_suitChars[SUIT(cardIndex)];
    *outString   = '\0';

    return 2;
}

int
Poker_maskToCards(void *cardMask, int cards[]) {
    int i, n=0;
    CardMask c = *((CardMask *) cardMask);

    for (i=POKER_N_CARDS-1; i >= 0; i--) 
        if (CardMask_CARD_IS_SET(c, i)) 
            cards[n++] = i;

    return n;
}

int
Poker_NumCards(void *cardMask) {
    CardMask c = *((CardMask *) cardMask);
    int i;
    int ncards = 0;
    for (i=0; i<POKER_N_CARDS; i++)
        if (CardMask_CARD_IS_SET(c, i))
            ncards++;
    return ncards;
}

Deck StdDeck = { 
    POKER_N_CARDS, 
    Poker_cardToString, 
    Poker_stringToCard,
    Poker_maskToCards,
    Poker_NumCards
};

int
GenericPoker_maskToString(Deck *deck, void *cardMask, char *outString) {
    int cards[STRING_CARDS], n, i;
    char *p;

    n = (*deck->maskToCards)(cardMask, cards);

    p = outString;
    for (i=0; i<n; i++) {
        if (i > 0) 
            *p++ = ' ';
        p += (*deck->cardToString)(cards[i], p);
    };
    *p = '\0';
    return (outString - p);
}

char *
GenericPoker_maskString(Deck *deck, void *cardMask) {
    static char outString[150];

    GenericPoker_maskToString(deck, cardMask, outString);
    return outString;
}

static void
parseArgs(int argc, char **argv) {
    int i, count = 0, c;

    for (i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-d") == 0) {
                if (++i == argc) goto error;
                if (Poker_stringToCard(argv[i], &c) == 0)
                    goto error;
                if (!CardMask_CARD_IS_SET(gDeadCards, c)) {
                    ++gNDead;
                    CardMask_SET(gDeadCards, c);
                };
            } 
            else 
                goto error;
        } else {
            if (Poker_stringToCard(argv[i], &c) == 0)
                goto error;
            if (count < 2) 
                CardMask_SET(gTestPlayerCards[0], c);
            /*else if (count < 4) 
                CardMask_SET(gTestPlayerCards[1], c);*/
            else {
                CardMask_SET(gCommonCards, c);
                ++gNCommon;
            };
            ++count;
        }
    }
    if (count < 4) goto error;
    if (gNCommon > 5) goto error;

    return;

error:
    fprintf(stderr, "Usage: hcmp2 [ -d dead-card ] p1-cards p2-cards [ common-cards ]\n");
    exit(0);
}

int test_main( int argc, char *argv[] )
{
  CardMask cards, p0, p1, c0, c1;
  uint32 h0, h1;
  int h0_count=0, h1_count=0, tie_count=0, count=0;

  CardMask_RESET(gDeadCards);
  CardMask_RESET(gCommonCards);
  CardMask_RESET(gTestPlayerCards[0]);
  CardMask_RESET(gTestPlayerCards[1]);
  parseArgs(argc, argv);

  CardMask_OR(p0, gTestPlayerCards[0], gCommonCards);
  CardMask_OR(p1, gTestPlayerCards[1], gCommonCards);
  CardMask_OR(gDeadCards, gDeadCards, gCommonCards);
  CardMask_OR(gDeadCards, gDeadCards, gTestPlayerCards[0]);
  CardMask_OR(gDeadCards, gDeadCards, gTestPlayerCards[1]);

  CardStatCount stat_count = CalcPlayerStatCount(gTestPlayerCards[0], gDeadCards, gCommonCards);

//   printf("%d boards", stat_count.total_count);
//     printf(" containing %s ", Poker_maskString(gCommonCards));
// printf("\n");
// 
//   printf("  cards      win  %%win       loss  %%lose       tie  %%tie      EV\n");
//   printf("  %s  %7d %6.2f   %7d %6.2f   %7d %6.2f     %5.3f\n", 
//       Poker_maskString(gTestPlayerCards[0]), 
//       stat_count.win_count, 100.0*stat_count.win_count/stat_count.total_count, 
//       stat_count.fail_count, 100.0*stat_count.fail_count/stat_count.total_count, 
//       stat_count.tie_count, 100.0*stat_count.tie_count/stat_count.total_count, 
//       (1.0*stat_count.win_count + (stat_count.tie_count/2.0)) / stat_count.total_count);

  /*
  CalcCardsRate(5-gNCommon, gDeadCards, p0, p1);

  if (gNCommon > 0) 
      printf(" containing %s ", Poker_maskString(gCommonCards));
  if (gNDead) 
      printf(" with %s removed ", Poker_maskString(gDeadCards));
  printf("\n");

  int type = GetCardsType(p0, 5);
  printf("type: %s => %d\n", Poker_maskString(p0),type );
  */
  exit(0);

  return 0;
}
