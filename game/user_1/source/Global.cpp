#include "Global.h"
#include "Player.h"
#include <map>

int g_small_blind_bet = 0;
int g_small_blind_pid = -1;
int g_big_blind_bet = 0;
int g_big_blind_pid = -1;
int g_residual_turn = 500;
int g_crruent_round = 0;

Play_Round_Cards gPlayerCards;
std::vector<Player> gAllPlayers;

std::map<std::string, int> msgPointCovertMap;

std::map<int,int> gAllCurRoundBet;

std::map<int,int> gPersonalityAnalyz;

uint32 gCurPotTotalbet;

BET_ROUND  gCurBetRound = NOT_BET_ROUND;

int g_init_glod = 0;

bool g_once_flag = true;

int g_cur_round_glod = 0;

bool g_naughties_flag = false;

//第一维为投注轮
//第二维单轮得到信息次数
//第三维所有玩家本轮投注信息
std::vector< std::vector< std::vector<InquirePlayerInfo> > >  gAllInquireMsgInfo;

void initPointCovert()
{
    msgPointCovertMap.insert(std::make_pair("2",Rank_2));
    msgPointCovertMap.insert(std::make_pair("3",Rank_3));
    msgPointCovertMap.insert(std::make_pair("4",Rank_4));
    msgPointCovertMap.insert(std::make_pair("5",Rank_5));
    msgPointCovertMap.insert(std::make_pair("6",Rank_6));
    msgPointCovertMap.insert(std::make_pair("7",Rank_7));
    msgPointCovertMap.insert(std::make_pair("8",Rank_8));
    msgPointCovertMap.insert(std::make_pair("9",Rank_9));
    msgPointCovertMap.insert(std::make_pair("10",Rank_TEN));
    msgPointCovertMap.insert(std::make_pair("J",Rank_JACK));
    msgPointCovertMap.insert(std::make_pair("Q",Rank_QUEEN));
    msgPointCovertMap.insert(std::make_pair("K",Rank_KING));
    msgPointCovertMap.insert(std::make_pair("A",Rank_ACE));
}


void InitGlobalData()
{
    CardMask_RESET(gPlayerCards.hand_cards);
    CardMask_RESET(gPlayerCards.common_cards);
    CardMask_RESET(gPlayerCards.turn_card);
    CardMask_RESET(gPlayerCards.river_card);

    initPointCovert();

    gCurPotTotalbet = 0;
}

void ClearRoundGlobalData()
{
    CardMask_RESET(gPlayerCards.hand_cards);
    CardMask_RESET(gPlayerCards.common_cards);
    CardMask_RESET(gPlayerCards.turn_card);
    CardMask_RESET(gPlayerCards.river_card);

    gCurPotTotalbet = 0;
    gAllPlayers.clear();
}

int CovertMsgPoint(const std::string &msg)
{
    if(msgPointCovertMap.count(msg) > 0)
    {
        return msgPointCovertMap[msg];
    }
    return -1;
}

uint32 CovertMsgSuit(const std::string &msg)
{
    uint32 suit;

    if(msg == "SPADES")
    {
        suit = Suit_SPADES;
    }
    else if (msg == "HEARTS")
    {
        suit = Suit_HEARTS;
    }
    else if (msg == "CLUBS")
    {
        suit = Suit_CLUBS;
    }
    else// if (msg == "DIAMONDS")
    {
        suit = Suit_DIAMONDS;
    }

    return suit;
}

void SavePlayer(PLAYER_TYPE type, int pid, uint32 money, std::string &jetton)
{
    //printf("save player:%d,%d,%d,%s\n", type, pid, money, jetton.c_str());

    Player new_player;

    new_player.type = type;
    new_player.pid = pid;
    new_player.money = money;
    new_player.jetton = jetton;

    if (new_player.pid == GetPlayerId())
    {
        g_cur_round_glod = money + atoi(jetton.c_str());
    }

    if(g_once_flag)
    {
        g_init_glod += money + atoi(jetton.c_str());
    }

    gAllPlayers.push_back(new_player);
}

void CleanPlayer()
{
    gAllPlayers.clear();

}

int GetMySelfJetton()
{
    std::vector<InquirePlayerInfo>::iterator it =  gAllInquireMsgInfo[gCurBetRound].back().begin();
    for (; it != gAllInquireMsgInfo[gCurBetRound].back().end(); ++it)
    {
        if (it->pid == GetPlayerId())
        {
            return it->jetton;
        }
    }

    std::vector<Player>::iterator itt =  gAllPlayers.begin();
    for (; itt != gAllPlayers.end(); ++itt)
    {
        if (itt->pid == GetPlayerId())
        {
            return atoi(itt->jetton.c_str());
        }
    }

    return 0;
}

int GetMySelfGlod()
{
    std::vector<InquirePlayerInfo>::iterator it =  gAllInquireMsgInfo[gCurBetRound].back().begin();
    for (; it != gAllInquireMsgInfo[gCurBetRound].back().end(); ++it)
    {
        if (it->pid == GetPlayerId())
        {
            return it->money;
        }
    }
    return g_cur_round_glod;
}

int GetCurrentPlayerNum()
{
    int player_num = 0;

    //为记录的玩家数
    int no_record_player_num = (int)(gAllPlayers.size() - gAllInquireMsgInfo[gCurBetRound].back().size());

    std::vector<InquirePlayerInfo>::iterator it =  gAllInquireMsgInfo[gCurBetRound].back().begin();
    for (; it != gAllInquireMsgInfo[gCurBetRound].back().end(); ++it)
    {
        if (it->action != "fold")
        {
            player_num++;
        }
    }
    return player_num + no_record_player_num;
}

int GetMySelfBet()
{
    std::vector<InquirePlayerInfo>::iterator it =  gAllInquireMsgInfo[gCurBetRound].back().begin();
    for (; it != gAllInquireMsgInfo[gCurBetRound].back().end(); ++it)
    {
        if (it->pid == GetPlayerId())
        {
            return it->bet;
        }
    }
    return 0;
}

//本手牌共下注的筹码数
int GetTotalBet()
{
    std::vector<InquirePlayerInfo>::iterator it =  gAllInquireMsgInfo[gCurBetRound].back().begin();
    for (; it != gAllInquireMsgInfo[gCurBetRound].back().end(); ++it)
    {
        if (it->pid == GetPlayerId())
        {
            return it->bet;
        }
    }
    
    return 0;
}

//自己的手中剩余筹码数
int GetRemainBet()
{
    std::vector<InquirePlayerInfo>::iterator it =  gAllInquireMsgInfo[gCurBetRound].back().begin();
    for (; it != gAllInquireMsgInfo[gCurBetRound].back().end(); ++it)
    {
        if (it->pid == GetPlayerId())
        {
            return it->jetton;
        }
    }

    std::vector<Player>::iterator itt =  gAllPlayers.begin();
    for (; itt != gAllPlayers.end(); ++itt)
    {
        if (itt->pid == GetPlayerId())
        {
            return atoi(itt->jetton.c_str());
        }
    }

    return 0;
}

int GetMinBet()
{
    int my_bet = 0;
    int other_max_bet = 0;
    std::vector<InquirePlayerInfo>::iterator it =  gAllInquireMsgInfo[gCurBetRound].back().begin();
    for (; it != gAllInquireMsgInfo[gCurBetRound].back().end(); ++it)
    {
        if (it->action != "fold")
        {
            if (it->bet > other_max_bet)
            {
                other_max_bet = it->bet;
            }
        }
        if (it->pid == GetPlayerId())
        {
            my_bet = it->bet;
        }
    }
    
    int min_bet = ((other_max_bet - my_bet) > 0 ? (other_max_bet - my_bet) : 0);

    int my_self_jetton = GetMySelfJetton();
    if (my_self_jetton < min_bet)
    {
        min_bet = my_self_jetton;
    }

    return min_bet;
}


int GetWinJetton(int min_bet)
{
    int total = 0;
    std::vector<InquirePlayerInfo>::iterator it =  gAllInquireMsgInfo[gCurBetRound].back().begin();
    for (; it != gAllInquireMsgInfo[gCurBetRound].back().end(); ++it)
    {
        if (it->bet > GetMySelfBet() + min_bet)
        {
            total += GetMySelfBet() + min_bet;
        }
        else
        {
            total += it->bet;
        }
    }
    LOG("my_self_bet=[%d],min_bet=[%d],total=[%d]\n",GetMySelfBet(),min_bet,total);
    return total;
}

int GetMyCurRoundPosition()
{
    int GetCurRoundBetPlayerNum = 0;
    int my_bet = 0;
    int other_max_bet = 0;
    std::vector<InquirePlayerInfo>::iterator it =  gAllInquireMsgInfo[gCurBetRound].back().begin();
    for (; it != gAllInquireMsgInfo[gCurBetRound].back().end(); ++it)
    {
        if (it->action != "fold")
        {
            if (it->bet > other_max_bet)
            {
                other_max_bet = it->bet;
            }
        }
    }

    it =  gAllInquireMsgInfo[gCurBetRound].back().begin();
    for (; it != gAllInquireMsgInfo[gCurBetRound].back().end(); ++it)
    {
        if (it->action != "fold")
        {
            if (it->bet < other_max_bet)
            {
                GetCurRoundBetPlayerNum++;
            }
        }
    }
    
    //LOG("!!!!!!!!!!GetCurRoundBetPlayerNum=[%d]!!!!!!!!!\n",GetCurRoundBetPlayerNum);
    return GetCurRoundBetPlayerNum;
}


PLAYER_ACTION GetPlayersAction()
{
    const std::vector<InquirePlayerInfo>&  last_player_info = gAllInquireMsgInfo[gCurBetRound].back();

    int fold_count = 0;
    int call_count = 0;
    int raise_count = 0;

    int player_count = (int)last_player_info.size();

    for (std::vector<InquirePlayerInfo>::const_iterator iter = last_player_info.begin();
        iter != last_player_info.end();
        ++iter)
    {
        if (iter->pid == GetPlayerId())
        {
            --player_count;
            continue;
        }

        if ("blind" == iter->action)
        {
            --player_count;
            continue;
        }

        if ("fold" == iter->action)
        {
            ++fold_count;
        }

        if ("call" == iter->action)
        {
            ++call_count;
        }

        if (("raise" == iter->action) || ("all_in" == iter->action))  // TODO: all in 的处理
        {
            ++raise_count;
        }
    }


    if (player_count == fold_count)
    {
        return ALL_FOLD;
    }

    if ((raise_count == 1) && (call_count == 0))
    {
        return ONE_RAISE_NO_CALL;
    }

    if ((raise_count == 1) && (ONE_RAISE_ONEMORE_CALL > 0))
    {
        return ONE_RAISE_ONEMORE_CALL;
    }

    if (1 == call_count)
    {
        return ONE_CALL;
    }

    if (call_count > 1)
    {
        return MORE_CALL;
    }

    //有超过1个玩家加注
    return MORE_RAISE;
}

MY_POSITION GetPlayerPosition()
{   
    int my_pos = 0;
    for (std::vector<Player>::const_iterator iter = gAllPlayers.begin(); 
        iter != gAllPlayers.end();
        ++iter)
    {
        if (iter->pid == GetPlayerId())
        {
            if (PLAYER_IS_SMALL_BLIND == iter->type)
            {
                return SMALL_BLIND_POS;
            }
            else if (PLAYER_IS_BIG_BLIND == iter->type)
            {
                return BIG_BLIND_POS;
            }
            else if (PLAYER_IS_BUTTON == iter->type)
            {
                return BEHIND_POS;
            }
            break;
        }
        ++my_pos;
    }

    int players = (int)gAllPlayers.size();
    if (players <= 3)
    {
        return BEHIND_POS;
    }

    if (players == 8)
    {
        if ((my_pos == 3) || (my_pos == 4))
        {
            return FRONT_POS;
        }
        else if ((my_pos == 5) || (my_pos == 6))
        {
            return CENTER_POS;
        }
        else
        {
            return BEHIND_POS;
        }
    }

    if (players == 7)
    {
        if ((my_pos == 3) || (my_pos == 4))
        {
            return FRONT_POS;
        }
        else if ((my_pos == 5))
        {
            return CENTER_POS;
        }
        else
        {
            return BEHIND_POS;
        }
    }

    if (players == 6)
    {
        if ((my_pos == 3))
        {
            return FRONT_POS;
        }
        else if ((my_pos == 4))
        {
            return CENTER_POS;
        }
        else
        {
            return BEHIND_POS;
        }
    }

    if (players == 5)
    {
        if ((my_pos == 3))
        {
            return FRONT_POS;
        }
        else
        {
            return CENTER_POS;
        }
    }

    if (players == 4)
    {
        return FRONT_POS;
    }

    return BEHIND_POS;
}

double GetMadnessRate(std::vector<int> &pids)
{
    int cur_round = 500 - g_residual_turn;
    if (cur_round < 0)
    {
        cur_round = 500 + (0-cur_round);
    }

    if (cur_round < 32 || pids.size() <= 0)
    {
        return 0;
    }

    double rate = 1;

    for(std::vector<int>::iterator it = pids.begin(); it != pids.end(); ++it)
    {
        double r = 0.1;
        int pid = *it;
        if (gPersonalityAnalyz.count(pid) > 0)
        {
            r = (double)gPersonalityAnalyz[pid] / (double)cur_round;
            rate = rate * r;
        }

        rate = rate * r;
    }

    return rate;
}

int GetPlayersTotalMoney(int pid)
{
    std::vector<Player>::iterator it = gAllPlayers.begin();
    for (; it != gAllPlayers.end(); ++it)
    {
        if (pid == it->pid)
        {
            return it->money + atoi(it->jetton.c_str());
        }
    }

    LOG("GetPlayersTotalMoney Failed! it's impossible. pid = %d.\n", pid);
    return 0;
}

//获取自己和其他玩家金币和的差距(金币包括金币和筹码)
//返回自己金币-其他玩家金币
int GetGoldGap()
{
    int my_gold = 0;
    int other_players_gold = 0;

    std::vector<Player>::iterator it = gAllPlayers.begin();
    for (; it != gAllPlayers.end(); ++it)
    {
        if (GetPlayerId() == it->pid)
        {
            my_gold =  it->money + atoi(it->jetton.c_str());
        }
        else
        {
            other_players_gold +=  it->money + atoi(it->jetton.c_str());
        }
    }

    return my_gold - other_players_gold;
}

// int GetOtherPlayersGold()
// {
//     int other_players_gold = 0;
// 
//     std::vector<Player>::iterator it = gAllPlayers.begin();
//     for (; it != gAllPlayers.end(); ++it)
//     {
//         if (GetPlayerId() != it->pid)
//         {
//             other_players_gold +=  it->money + atoi(it->jetton.c_str());
//         }
//     }
// 
//     return other_players_gold;
// }

bool MustWin()
{
    int gold_gap = GetGoldGap();
    if (gold_gap > 0)
    {
        int fold_golds = 0;
        if (gAllPlayers.size() == 2)
        {
            fold_golds = ((g_residual_turn / 2) + 1) * g_small_blind_bet;
        }
        else
        {
            fold_golds = ((g_residual_turn / gAllPlayers.size()) + 1) * (g_small_blind_bet + g_big_blind_bet);
        }

        if (gold_gap >= fold_golds * 2)
        {
            LOG("Winwinwinwin!!!!!gold_gap = %d, players = %d, g_residual_turn = %d.\n");
            return true;            
        }
    }
    return false;
}