#include "ChooseStrategy.h"
#include "StrategySwitch.h"
#include "CalcRR.h"
#include "Player.h"
#include <stdlib.h>

bool GoldProtectStrategy()
{
    int my_gold = 0;
    int other_player_gold = 0;
    if (gAllPlayers.size() <= 2)
    {
        std::vector<Player>::iterator it = gAllPlayers.begin();
        for (; it != gAllPlayers.end(); ++it)
        {
            if (GetPlayerId() == it->pid)
            {
                my_gold =  it->money + atoi(it->jetton.c_str());
            }
            else
            {
                other_player_gold =  it->money + atoi(it->jetton.c_str());
            }
        }

        if (my_gold - other_player_gold > GOLD_PROTECT)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
    
}

void FreeLookPoker(int min_bet,StrategyInof* strategy_info)
{
    if (min_bet == 0)
    {
        strategy_info->action = ACTION_CHECK;
    }
    else
    {
        strategy_info->action = ACTION_FOLD;
    }
}


bool NaughtiesPoker(int min_bet,StrategyInof* strategy_info)
{ 
    
    if (min_bet == 0 && GetMyCurRoundPosition() == 1 && gCurBetRound == FLOP_BET_ROUND && !g_naughties_flag)
    {
        LOG("!!!!!!!!!!NaughtiesPoker!!!!!!!!!\n");
        strategy_info->action = ACTION_RAISE; 
        strategy_info->money = g_big_blind_bet;
        g_naughties_flag = true;
        return true;
    }
    return false;
}

void lowLevelFilter(StrategyInof &strategy_info, double hand_strength, int min_bet, int my_self_bet)
{
    if(strategy_info.action == ACTION_CALL)
    {
        int beishu = min_bet / my_self_bet;
        LOG("LevelFilter:hand_strength:%f,min_bet:%d,my_self_bet%d\n", hand_strength, min_bet, my_self_bet);
        if (beishu == 3)
        {
            if(hand_strength < 75)
            {
                FreeLookPoker(min_bet,&strategy_info);
                LOG("LevelFilter75\n");
            }
        }
        else
        {
            if (beishu > 3)
            {
                if(hand_strength < 85)
                {
                    FreeLookPoker(min_bet,&strategy_info);
                    LOG("LevelFilter85\n");
                }
            }

        }
    }
}

StrategyInof ChooseStrategy()
{
    StrategyInof strategy_info;
    strategy_info.action = ACTION_CHECK;

    int min_bet = GetMinBet();

    double hand_strength = CalcHandStrength();
    double RR = CalcRR(min_bet,hand_strength);

    size_t player_num = gAllPlayers.size();
    if (player_num > 3)
    {
 	    RR -= 0.4;
    }

    LOG("hand_strength=[%lf],RR=[%lf],gCurBetRound=[%d],CurrentPlayerNum=[%d]\n",hand_strength,RR,gCurBetRound,GetCurrentPlayerNum());

    int raise_double_bet = g_big_blind_bet * 2;

    int raise_Three_bet = g_big_blind_bet * 3;

    int raise_Four_bet = g_big_blind_bet * 4;

    int num_rand = RAND;

    int my_self_bet = GetMySelfBet() ;

    int my_self_total_Jetton = GetMySelfJetton() + my_self_bet;

    if (hand_strength >= 0.999999)
    {
        strategy_info.action = ACTION_ALL_IN;
    }

    //直接弃牌

    if (RR < 0.5)
    {
        FreeLookPoker(min_bet,&strategy_info);
    }

    //如果RR<0.8，那么95%选择弃牌，0%选择叫牌，5%选择加倍（这里加倍的目的是为了虚张声势）
    else if (RR < 0.8)
    {
        //if (!NaughtiesPoker(min_bet,&strategy_info))
        {
            if (num_rand < 95 || gCurBetRound == RIVER_BET_ROUND || gCurBetRound == TURN_BET_ROUND)
            {
                //flod
                FreeLookPoker(min_bet,&strategy_info);

            }
            else
            {
                //call
                strategy_info.action = ACTION_CALL;
                //strategy_info.money = raise_double_bet;   
            }
        }
        
    }
    //如果RR<1.0,那么80%选择弃牌，5%选择叫牌，15%选择加倍（虚张声势，虚张声势！！！）
    else if (RR < 1.0)
    {
        if (!NaughtiesPoker(min_bet,&strategy_info))
        {
            if (num_rand < 80 || gCurBetRound == RIVER_BET_ROUND || gCurBetRound == TURN_BET_ROUND)
            {
                FreeLookPoker(min_bet,&strategy_info);

            }
            //         else if (num_rand >= 80 && num_rand < 95)
            //         {
            //             //double call
            //             strategy_info.action = ACTION_RAISE;
            //             strategy_info.money = raise_double_bet;
            //         }
            else
            {
                strategy_info.action = ACTION_CALL;
            }
        }
    }
    //如果RR<1.3，那么0%选择弃牌，60%选择叫牌，40%选择加倍
    else if (RR < 1.3)
    {
//         if (num_rand < 60 || GetMyCurRoundPosition() > 2 )
//         {
            strategy_info.action = ACTION_CALL;
//         }
//         else
//         {
//             strategy_info.action = ACTION_RAISE;
//             strategy_info.money = raise_double_bet;
//         }
    }
    //另外，如果RR<1.6，那么0%选择弃牌，30%选择叫牌，70%选择加倍
    else
    {
        //勾引
        if (gCurBetRound == FLOP_BET_ROUND || gCurBetRound == TURN_BET_ROUND)
        {
            
            if (hand_strength < 0.65 || GetMyCurRoundPosition() > 2)
            {
                strategy_info.action = ACTION_CALL;
            }
            else
            {
                double bet_percent = (double)(my_self_bet + raise_double_bet) / my_self_total_Jetton;
                //procet
                if (GoldProtectStrategy())
                {
                    if (bet_percent > 0.2)
                    {
                        strategy_info.action = ACTION_CALL;
                    }
                    else
                    {
                        strategy_info.action = ACTION_RAISE;
                        strategy_info.money = raise_double_bet;
                    }
                }
                else
                {
                    if (bet_percent > 0.3)
                    {
                        strategy_info.action = ACTION_CALL;
                    }
                    else
                    {
                        strategy_info.action = ACTION_RAISE;
                        strategy_info.money = raise_double_bet;
                    }
                }
                
            }
        }
        else if (gCurBetRound == RIVER_BET_ROUND)
        {
            if (hand_strength < 0.6)
            {
                strategy_info.action = ACTION_CALL;
            }
            else if (hand_strength < 0.7)
            {
                double bet_percent = (double)(my_self_bet + raise_double_bet) / my_self_total_Jetton;
                if (GoldProtectStrategy())
                {
                    if (bet_percent > 0.2)
                    {
                        strategy_info.action = ACTION_CALL;
                    }
                    else
                    {
                        strategy_info.action = ACTION_RAISE;
                        strategy_info.money = raise_double_bet;
                    }
                }
                else
                {
                    if (bet_percent > 0.4)
                    {
                        strategy_info.action = ACTION_CALL;
                    }
                    else
                    {
                        strategy_info.action = ACTION_RAISE;
                        strategy_info.money = raise_double_bet;
                    }
                }
            }
            else if (hand_strength < 0.8)
            {
                double bet_percent = (double)(my_self_bet + raise_Three_bet) / my_self_total_Jetton;
                if (GoldProtectStrategy())
                {
                    if (bet_percent > 0.3)
                    {
                        strategy_info.action = ACTION_CALL;
                    }
                    else
                    {
                        strategy_info.action = ACTION_RAISE;
                        strategy_info.money = raise_Three_bet;
                    }
                }
                else
                {
                    if (bet_percent > 0.5)
                    {
                        strategy_info.action = ACTION_CALL;
                    }
                    else
                    {
                        strategy_info.action = ACTION_RAISE;
                        strategy_info.money = raise_Three_bet;
                    }
                }
            }
            else if (hand_strength < 0.9)
            {
                double bet_percent = (double)(my_self_bet + raise_Four_bet) / my_self_total_Jetton;
                if (GoldProtectStrategy())
                {
                    if (bet_percent > 0.5)
                    {
                        strategy_info.action = ACTION_CALL;
                    }
                    else
                    {
                        strategy_info.action = ACTION_RAISE;
                        strategy_info.money = raise_Four_bet;
                    }             
                }
                else
                {
                    if (bet_percent > 0.7)
                    {
                        strategy_info.action = ACTION_CALL;
                    }
                    else
                    {
                        strategy_info.action = ACTION_RAISE;
                        strategy_info.money = raise_Four_bet;
                    }  
                }

            }
            else
            {
                double bet_percent = (double)(my_self_bet + raise_Four_bet) / my_self_total_Jetton;
                if (GoldProtectStrategy())
                {
                    if (bet_percent > 0.7)
                    {
                        strategy_info.action = ACTION_CALL;
                    }
                    else
                    {
                        strategy_info.action = ACTION_RAISE;
                        strategy_info.money = raise_Four_bet;
                    }             
                }
                else
                {
                    strategy_info.action = ACTION_RAISE;
                    strategy_info.money = raise_Four_bet;
                }

            }
        }
    }

    lowLevelFilter(strategy_info, hand_strength, min_bet, my_self_bet);
    
    return strategy_info;
}





