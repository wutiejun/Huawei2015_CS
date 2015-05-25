#ifndef _CHOOSE_STRATEGY_H_
#define _CHOOSE_STRATEGY_H_
#include "Global.h"
typedef struct _StrategyInof
{
    Action action;
    int    money;
}StrategyInof;

StrategyInof ChooseStrategy();

void FreeLookPoker(int min_bet,StrategyInof* strategy_info);

#endif