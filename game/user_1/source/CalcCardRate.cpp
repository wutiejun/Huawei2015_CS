#include <stdio.h>
#include "CalcCardRate.h"
#include "test_poker.h"

static inline uint32 
Eval_Cards_val( CardMask cards, int n_cards )
{
  uint32 retval;
  uint32 ranks, four_mask, three_mask, two_mask, 
    n_dups, n_ranks;
  uint32 sc, sd, sh, ss;

  ss = CardMask_SPADES(cards);
  sc = CardMask_CLUBS(cards);
  sd = CardMask_DIAMONDS(cards);
  sh = CardMask_HEARTS(cards);

  retval = 0;
  ranks = sc | sd | sh | ss;
  n_ranks = nBitsTable[ranks];
  n_dups = n_cards - n_ranks;

  /* 如果是顺子，同花，同花顺，立即返回
  */
  if (n_ranks >= 5) {
    if (nBitsTable[ss] >= 5) {
      if (straightTable[ss]) 
        return HandVal_HANDTYPE_VALUE(HandType_STFLUSH)
          + HandVal_TOP_CARD_VALUE(straightTable[ss]);
      else
        retval = HandVal_HANDTYPE_VALUE(HandType_FLUSH) 
          + topFiveCardsTable[ss];
    } 
    else if (nBitsTable[sc] >= 5) {
      if (straightTable[sc]) 
        return HandVal_HANDTYPE_VALUE(HandType_STFLUSH)
          + HandVal_TOP_CARD_VALUE(straightTable[sc]);
      else 
        retval = HandVal_HANDTYPE_VALUE(HandType_FLUSH) 
          + topFiveCardsTable[sc];
    } 
    else if (nBitsTable[sd] >= 5) {
      if (straightTable[sd]) 
        return HandVal_HANDTYPE_VALUE(HandType_STFLUSH)
          + HandVal_TOP_CARD_VALUE(straightTable[sd]);
      else 
        retval = HandVal_HANDTYPE_VALUE(HandType_FLUSH) 
          + topFiveCardsTable[sd];
    } 
    else if (nBitsTable[sh] >= 5) {
      if (straightTable[sh]) 
        return HandVal_HANDTYPE_VALUE(HandType_STFLUSH)
          + HandVal_TOP_CARD_VALUE(straightTable[sh]);
      else 
        retval = HandVal_HANDTYPE_VALUE(HandType_FLUSH) 
          + topFiveCardsTable[sh];
    } 
    else {
      int st;

      st = straightTable[ranks];
      if (st) 
        retval = HandVal_HANDTYPE_VALUE(HandType_STRAIGHT)
          + HandVal_TOP_CARD_VALUE(st);
    };

    if (retval && n_dups < 3)
      return retval;
  };

  /*
   * 有两种情况: 
     1) 没有顺子或者同花
     2) 有顺子或者同花，还可能有葫芦和四条
   */
  switch (n_dups)
    {
    case 0:
      /* It's a high card */
      return HandVal_HANDTYPE_VALUE(HandType_NOPAIR)
        + topFiveCardsTable[ranks];
      break;
      
    case 1: {
      /* It's a one pair */
      uint32 t, kickers;

      two_mask   = ranks ^ (sc ^ sd ^ sh ^ ss);

      retval = HandVal_HANDTYPE_VALUE(HandType_ONEPAIR)
        + HandVal_TOP_CARD_VALUE(topCardTable[two_mask]);
      t = ranks ^ two_mask;      
      kickers = (topFiveCardsTable[t] >> HandVal_CARD_WIDTH)
        & ~HandVal_FIFTH_CARD_MASK;
      retval += kickers;

      return retval;
    }
    break;
      
    case 2: 
      /* it's a two pair */

      two_mask   = ranks ^ (sc ^ sd ^ sh ^ ss);
      if (two_mask) { 
        uint32 t;

        t = ranks ^ two_mask; 
        retval = HandVal_HANDTYPE_VALUE(HandType_TWOPAIR)
          + (topFiveCardsTable[two_mask]
             & (HandVal_TOP_CARD_MASK | HandVal_SECOND_CARD_MASK))
          + HandVal_THIRD_CARD_VALUE(topCardTable[t]);

        return retval;
      }
      else {
        int t, second;
        
        three_mask = (( sc&sd )|( sh&ss )) & (( sc&sh )|( sd&ss ));
        
        retval = HandVal_HANDTYPE_VALUE(HandType_TREEKIND)
          + HandVal_TOP_CARD_VALUE(topCardTable[three_mask]);

        t = ranks ^ three_mask; 
        second = topCardTable[t];
        retval += HandVal_SECOND_CARD_VALUE(second);
        t ^= (1 << second);
        retval += HandVal_THIRD_CARD_VALUE(topCardTable[t]);
        return retval;
      }
      break;
      
    default:
      /* may be four of a kind, full house, straight or flush, or two pair */
      four_mask  = sh & sd & sc & ss;
      if (four_mask) {
        int tc;

        tc = topCardTable[four_mask];
        retval = HandVal_HANDTYPE_VALUE(HandType_FOURKIND)
          + HandVal_TOP_CARD_VALUE(tc)
          + HandVal_SECOND_CARD_VALUE(topCardTable[ranks ^ (1 << tc)]);
        return retval;
      };


      two_mask   = ranks ^ (sc ^ sd ^ sh ^ ss);
      if (nBitsTable[two_mask] != n_dups) {
        int tc, t;

        three_mask = (( sc&sd )|( sh&ss )) & (( sc&sh )|( sd&ss ));
        retval  = HandVal_HANDTYPE_VALUE(HandType_FULLHOUSE);
        tc = topCardTable[three_mask];
        retval += HandVal_TOP_CARD_VALUE(tc);
        t = (two_mask | three_mask) ^ (1 << tc);
        retval += HandVal_SECOND_CARD_VALUE(topCardTable[t]);
        return retval;
      };

      if (retval) /* flush and straight */
        return retval;
      else {
        /* Must be two pair */
        int top, second;
          
        retval = HandVal_HANDTYPE_VALUE(HandType_TWOPAIR);
        top = topCardTable[two_mask];
        retval += HandVal_TOP_CARD_VALUE(top);
        second = topCardTable[two_mask ^ (1 << top)];
        retval += HandVal_SECOND_CARD_VALUE(second);
        retval += HandVal_THIRD_CARD_VALUE(topCardTable[ranks ^ (1 << top) 
                                                        ^ (1 << second)]);
        return retval;
      };

      break;
    };

    return 0;
}
    

int 
GetCardsType( CardMask cards, int n_cards )
{
    uint32 ranks, four_mask, three_mask, two_mask, 
        n_dups, n_ranks, is_st_or_fl = 0, t, sc, sd, sh, ss;

    sc = CardMask_CLUBS(cards);
    sd = CardMask_DIAMONDS(cards);
    sh = CardMask_HEARTS(cards);
    ss = CardMask_SPADES(cards);

    ranks = sc | sd | sh | ss;
    n_ranks = nBitsAndStrTable[ranks] >> 2;
    n_dups = n_cards - n_ranks;

    if (nBitsAndStrTable[ranks] & 0x01) { /* if n_ranks > 5 */
        if (nBitsAndStrTable[ranks] & 0x02)
            is_st_or_fl = HandType_STRAIGHT;

        t = nBitsAndStrTable[ss] | nBitsAndStrTable[sc]
        | nBitsAndStrTable[sd] | nBitsAndStrTable[sh];

        if (t & 0x01) {
            if (t & 0x02) 
                return HandType_STFLUSH;
            else 
                is_st_or_fl = HandType_FLUSH;
        };

        if (is_st_or_fl && n_dups < 3)
            return is_st_or_fl;
    };

    switch (n_dups) {
  case 0:
      return HandType_NOPAIR;
      break;

  case 1:
      return HandType_ONEPAIR;
      break;

  case 2:
      two_mask = ranks ^ (sc ^ sd ^ sh ^ ss);
      return (two_mask != 0) 
          ? HandType_TWOPAIR 
          : HandType_TREEKIND;
      break;

  default:
      four_mask  = (sc & sd) & (sh & ss);
      if (four_mask) 
          return HandType_FOURKIND;
      three_mask = (( sc&sd )|( sh&ss )) & (( sc&sh )|( sd&ss ));
      if (three_mask) 
          return HandType_FULLHOUSE;
      else if (is_st_or_fl)
          return is_st_or_fl;
      else 
          return HandType_TWOPAIR;

      break;
    };

}

static CardStatCount 
Eval_Player_Stat(int n_cards, uint32 hand_value, CardMask dead_cards, CardMask other_player, uint32 n_rank)
{               
    CardStatCount stat_count = {0};

    uint32 other_h1;
    CardMask temp_c;

    int _i1; int _i2; int _i3;						
    int _i4; int _i5; int _i6;						
    int _i7; int _i8; int _i9;						
    CardMask _card1; CardMask _card2; CardMask _card3; 
    CardMask _card4; CardMask _card5; CardMask _card6; 
    CardMask _card7; CardMask _card8; CardMask _card9; 
    CardMask _n1; CardMask _n2; CardMask _n3;	
    CardMask _n4; CardMask _n5; CardMask _n6;	
    CardMask _n7; CardMask _n8; CardMask _n9;	

    _i1 = _i2 = _i3 = _i4 = _i5 = _i6 = _i7 = _i8 = _i9 = 0;                 
    CardMask_RESET(_card9);                                           
    _card1 = _card2 = _card3 = _card4 = _card5 = _card6                      
        = _card7 = _card8 = _card9;                                            
    CardMask_RESET(_n9);                                              
    _n1 = _n2 = _n3 = _n4 = _n5 = _n6 = _n7 = _n8 = _n9;                     

    switch (n_cards) {                                                       
  default:                                                                 
  case 9:                                                                  
  case 0:                                                                  
      break;                                                                 
  case 8:                                                                  
      _i2 = POKER_N_CARDS-1;                                                
      break;                                                                 
  case 7:                                                                  
      _i3 = POKER_N_CARDS-1;                                                
      break;                                                                 
  case 6:                                                                  
      _i4 = POKER_N_CARDS-1;                                                
      break;                                                                 
  case 5:                                                                  
      _i5 = POKER_N_CARDS-1;                                                
      break;                                                                 
  case 4:                                                                  
      _i6 = POKER_N_CARDS-1;                                                
      break;                                                                 
  case 3:                                                                  
      _i7 = POKER_N_CARDS-1;                                                
      break;                                                                 
  case 2:                                                                  
      _i8 = POKER_N_CARDS-1;                                                
      break;                                                                 
  case 1:                                                                  
      _i9 = POKER_N_CARDS-1;                                                
      break;                                                                 
    }                                                                        
    switch (n_cards) {                                                       
  default:                                                                 
      //printf("calc_card_rate: cards's number error!\n"); 
      break;                                                                 

  case 9:                                                                  
      for (_i1 = POKER_N_CARDS-1; _i1 >= 0; _i1--) {                        
          _card1 = POKER_MASK(_i1);                                           
          if (CardMask_ANY_SET(dead_cards, _card1))                     
              continue;                                                          
          _n1 = _card1;                                                        
          for (_i2 = _i1-1; _i2 >= 0; _i2--) {                                 
  case 8:                                                                  
      _card2 = POKER_MASK(_i2);                                         
      if (CardMask_ANY_SET(dead_cards, _card2))                   
          continue;                                                        
      CardMask_OR(_n2, _n1, _card2);                              
      for (_i3 = _i2-1; _i3 >= 0; _i3--) {                               
  case 7:                                                                  
      _card3 = POKER_MASK(_i3);                                       
      if (CardMask_ANY_SET(dead_cards, _card3))                 
          continue;                                                      
      CardMask_OR(_n3, _n2, _card3);                            
      for (_i4 = _i3-1; _i4 >= 0; _i4--) {                             
  case 6:                                                                  
      _card4 = POKER_MASK(_i4);                                     
      if (CardMask_ANY_SET(dead_cards, _card4))               
          continue;                                                    
      CardMask_OR(_n4, _n3, _card4);                          
      for (_i5 = _i4-1; _i5 >= 0; _i5--) {                           
  case 5:                                                                  
      _card5 = POKER_MASK(_i5);                                   
      if (CardMask_ANY_SET(dead_cards, _card5))             
          continue;                                                  
      CardMask_OR(_n5, _n4, _card5);                        
      for (_i6 = _i5-1; _i6 >= 0; _i6--) {                         
  case 4:                                                                  
      _card6 = POKER_MASK(_i6);                                 
      if (CardMask_ANY_SET(dead_cards, _card6))           
          continue;                                                
      CardMask_OR(_n6, _n5, _card6);                      
      for (_i7 = _i6-1; _i7 >= 0; _i7--) {                       
  case 3:                                                                  
      _card7 = POKER_MASK(_i7);                               
      if (CardMask_ANY_SET(dead_cards, _card7))         
          continue;                                              
      CardMask_OR(_n7, _n6, _card7);                    
      for (_i8 = _i7-1; _i8 >= 0; _i8--) {                     
  case 2:                                                                  
      _card8 = POKER_MASK(_i8);                             
      if (CardMask_ANY_SET(dead_cards, _card8))       
          continue;                                            
      CardMask_OR(_n8, _n7, _card8);                  
      for (_i9 = _i8-1; _i9 >= 0; _i9--) {                   
  case 1:                                                                  
      _card9 = POKER_MASK(_i9);                           
      if (CardMask_ANY_SET(dead_cards, _card9))     
          continue;                                          
      CardMask_OR(_n9, _n8, _card9);                
  case 0:
      {
          ++stat_count.total_count;
          CardMask_OR(temp_c, other_player, _n9);
          other_h1 = Eval_Cards_val(temp_c, n_rank);

          if (hand_value > other_h1)
              ++stat_count.win_count;
          else if (other_h1 > hand_value)
              ++stat_count.fail_count;
          else
              ++stat_count.tie_count;
      } 
      }                                                      
      }                                                        
      }                                                          
      }                                                            
      }                                                              
      }                                                                
      }                                                                  
          }                                                                    
      }                                                                      
    } 

    return stat_count;
}            

uint32 GetPlayerCardsCount(CardMask common_cards)
{
    uint32 ss = CardMask_SPADES(common_cards);
    uint32 sc = CardMask_CLUBS(common_cards);
    uint32 sd = CardMask_DIAMONDS(common_cards);
    uint32 sh = CardMask_HEARTS(common_cards);

    uint32 n_ranks = nBitsTable[ss] + nBitsTable[sc] + nBitsTable[sd] + nBitsTable[sh];

    return n_ranks;
}

CardStatCount 
CalcPlayerStatCount(CardMask player_cards, CardMask dead_cards, CardMask common_cards)
{
    CardMask p0 ;

    CardMask_OR(p0, player_cards, common_cards);

    uint32 ss = CardMask_SPADES(common_cards);
    uint32 sc = CardMask_CLUBS(common_cards);
    uint32 sd = CardMask_DIAMONDS(common_cards);
    uint32 sh = CardMask_HEARTS(common_cards);

    uint32 n_ranks = GetPlayerCardsCount(common_cards) + 2;
    //printf("n_ranks:%d =>%s\n", n_ranks, Poker_maskString(common_cards));
    
    uint32 hand_value = Eval_Cards_val(p0, n_ranks);

    return Eval_Player_Stat(2, hand_value, dead_cards, common_cards, n_ranks);
}

CardStatCount CalcCardsRate(int n_cards, CardMask dead_cards, CardMask p0, CardMask p1)
{               
    CardStatCount stat_count = {0};

    uint32 h0, h1;
    CardMask c0, c1;

    int _i1; int _i2; int _i3;						
    int _i4; int _i5; int _i6;						
    int _i7; int _i8; int _i9;						
    CardMask _card1; CardMask _card2; CardMask _card3; 
    CardMask _card4; CardMask _card5; CardMask _card6; 
    CardMask _card7; CardMask _card8; CardMask _card9; 
    CardMask _n1; CardMask _n2; CardMask _n3;	
    CardMask _n4; CardMask _n5; CardMask _n6;	
    CardMask _n7; CardMask _n8; CardMask _n9;	

    _i1 = _i2 = _i3 = _i4 = _i5 = _i6 = _i7 = _i8 = _i9 = 0;                 
    CardMask_RESET(_card9);                                           
    _card1 = _card2 = _card3 = _card4 = _card5 = _card6                      
        = _card7 = _card8 = _card9;                                            
    CardMask_RESET(_n9);                                              
    _n1 = _n2 = _n3 = _n4 = _n5 = _n6 = _n7 = _n8 = _n9;                     

    switch (n_cards) {                                                       
  default:                                                                 
  case 9:                                                                  
  case 0:                                                                  
      break;                                                                 
  case 8:                                                                  
      _i2 = POKER_N_CARDS-1;                                                
      break;                                                                 
  case 7:                                                                  
      _i3 = POKER_N_CARDS-1;                                                
      break;                                                                 
  case 6:                                                                  
      _i4 = POKER_N_CARDS-1;                                                
      break;                                                                 
  case 5:                                                                  
      _i5 = POKER_N_CARDS-1;                                                
      break;                                                                 
  case 4:                                                                  
      _i6 = POKER_N_CARDS-1;                                                
      break;                                                                 
  case 3:                                                                  
      _i7 = POKER_N_CARDS-1;                                                
      break;                                                                 
  case 2:                                                                  
      _i8 = POKER_N_CARDS-1;                                                
      break;                                                                 
  case 1:                                                                  
      _i9 = POKER_N_CARDS-1;                                                
      break;                                                                 
    }                                                                        
    switch (n_cards) {                                                       
  default:                                                                 
      //printf("calc_card_rate: cards's number error!\n"); 
      break;                                                                 

  case 9:                                                                  
      for (_i1 = POKER_N_CARDS-1; _i1 >= 0; _i1--) {                        
          _card1 = POKER_MASK(_i1);                                           
          if (CardMask_ANY_SET(dead_cards, _card1))                     
              continue;                                                          
          _n1 = _card1;                                                        
          for (_i2 = _i1-1; _i2 >= 0; _i2--) {                                 
  case 8:                                                                  
      _card2 = POKER_MASK(_i2);                                         
      if (CardMask_ANY_SET(dead_cards, _card2))                   
          continue;                                                        
      CardMask_OR(_n2, _n1, _card2);                              
      for (_i3 = _i2-1; _i3 >= 0; _i3--) {                               
  case 7:                                                                  
      _card3 = POKER_MASK(_i3);                                       
      if (CardMask_ANY_SET(dead_cards, _card3))                 
          continue;                                                      
      CardMask_OR(_n3, _n2, _card3);                            
      for (_i4 = _i3-1; _i4 >= 0; _i4--) {                             
  case 6:                                                                  
      _card4 = POKER_MASK(_i4);                                     
      if (CardMask_ANY_SET(dead_cards, _card4))               
          continue;                                                    
      CardMask_OR(_n4, _n3, _card4);                          
      for (_i5 = _i4-1; _i5 >= 0; _i5--) {                           
  case 5:                                                                  
      _card5 = POKER_MASK(_i5);                                   
      if (CardMask_ANY_SET(dead_cards, _card5))             
          continue;                                                  
      CardMask_OR(_n5, _n4, _card5);                        
      for (_i6 = _i5-1; _i6 >= 0; _i6--) {                         
  case 4:                                                                  
      _card6 = POKER_MASK(_i6);                                 
      if (CardMask_ANY_SET(dead_cards, _card6))           
          continue;                                                
      CardMask_OR(_n6, _n5, _card6);                      
      for (_i7 = _i6-1; _i7 >= 0; _i7--) {                       
  case 3:                                                                  
      _card7 = POKER_MASK(_i7);                               
      if (CardMask_ANY_SET(dead_cards, _card7))         
          continue;                                              
      CardMask_OR(_n7, _n6, _card7);                    
      for (_i8 = _i7-1; _i8 >= 0; _i8--) {                     
  case 2:                                                                  
      _card8 = POKER_MASK(_i8);                             
      if (CardMask_ANY_SET(dead_cards, _card8))       
          continue;                                            
      CardMask_OR(_n8, _n7, _card8);                  
      for (_i9 = _i8-1; _i9 >= 0; _i9--) {                   
  case 1:                                                                  
      _card9 = POKER_MASK(_i9);                           
      if (CardMask_ANY_SET(dead_cards, _card9))     
          continue;                                          
      CardMask_OR(_n9, _n8, _card9);                
  case 0:                                                                                                    
      {
          ++stat_count.total_count;

          CardMask_OR(c0, p0, _n9);
          h0 = Eval_Cards_val(c0, 7);
          CardMask_OR(c1, p1, _n9);
          h1 = Eval_Cards_val(c1, 7);
          //printf("c0:%s\n", Poker_maskString(c0));
          //printf("c1:%s\n", Poker_maskString(c1));
          if (h0 > h1)
              ++stat_count.win_count;
          else if (h1 > h0)
              ++stat_count.fail_count;
          else
              ++stat_count.tie_count;
      }                                       
      }                                                      
      }                                                        
      }                                                          
      }                                                            
      }                                                              
      }                                                                
      }                                                                  
          }                                                                    
      }                                                                      
    }        

    return stat_count;
} 

CardStatCount 
CalcPlayerStatCount_E(CardMask player_cards, CardMask dead_cards, CardMask common_cards)
{
    CardStatCount ret_stat_count = {0};

    CardMask p0, p1, pDead ;

    uint32 n_ranks = GetPlayerCardsCount(common_cards);
    //printf("n_ranks number:%d\n", n_ranks);
    if (n_ranks < 3)
    {
        //printf("-------------n_ranks error!!!!!!!!!!!\n");
        return ret_stat_count;
    }
    
    CardMask_OR(p0, player_cards, common_cards);

    int _i8, _i9;
    CardMask _card8; CardMask _card9;

    CardMask_RESET(_card9);                                              
    _card8 = _card9;  
    for (_i8 = POKER_N_CARDS-1; _i8 >= 0; _i8--) 
    {                                                                      
      _card8 = POKER_MASK(_i8);                             
      if (CardMask_ANY_SET(dead_cards, _card8))       
          continue;                                            
                 
      for (_i9 = _i8-1; _i9 >= 0; _i9--)
      {                   
          _card9 = POKER_MASK(_i9);                           
        if (CardMask_ANY_SET(dead_cards, _card9))     
            continue;                                          
        CardMask_OR(_card9, _card8, _card9);     

        CardMask_OR(p1, _card9, common_cards);

        CardMask_OR(pDead, p1, dead_cards);

        
        CardStatCount stat_count = CalcCardsRate(5-n_ranks, pDead, p0, p1);

        ret_stat_count.total_count += stat_count.total_count;
        ret_stat_count.win_count += stat_count.win_count;
        ret_stat_count.fail_count += stat_count.fail_count;
        ret_stat_count.tie_count += stat_count.tie_count;

      }
    }

    return ret_stat_count;
}