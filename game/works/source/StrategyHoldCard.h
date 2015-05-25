#ifndef _STRATEGY_HOLD_CARD_H_
#define _STRATEGY_HOLD_CARD_H_

#include <string>
#include "ChooseStrategy.h"

typedef struct  
{
    std::string color;
    std::string point;
}SingleCard;

//µ×ÅÆ
extern SingleCard g_holdcard[2];

StrategyInof GetHoldCardAction();

#endif