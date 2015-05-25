#include "StrategyHoldCard.h"
#include "Global.h"
#include "Player.h"

//底牌,收到底牌信息之后该数组被初始化

typedef enum
{
    VERY_STRENGTH = 0,   //超强牌
    STRENGTH,            //强牌
    NORMAL,              //普通牌
    SPECULARION,         //投机牌
    MIXED,               //混合牌
    STEAL_CARD,          //偷注牌
    FOLD_CARD,           //丢弃牌
    CARD_STRENGTH_COUNT
}CARD_STRENGTH;

SingleCard g_holdcard[2];

// static Action s_hold_card_action_tab[CARD_STRENGTH_COUNT][PLAYER_ACTION_COUNT][MY_POSITION_COUNT] = 
// {
//     //超强牌
//     {   //靠前         //居中        //靠后        //小盲注      //大盲注
//         {ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
//         {ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
//         {ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
//         {ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
//         {ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
//         {ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
//     },
// 
//     //强牌
//     {
//         {ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
//         {ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
//         {ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
//         {ACTION_FOLD,  ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
//         {ACTION_CALL,  ACTION_CALL,  ACTION_CALL,  ACTION_CALL,  ACTION_CALL},
//         {ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
//     },
// 
//     //中等牌
//     {
//         {ACTION_FOLD, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
//         {ACTION_FOLD, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
//         {ACTION_FOLD, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
//         {ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD,  ACTION_CALL},
//         {ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD,  ACTION_CALL}, //TODO:KQs跟注
//         {ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
//     },
// 
//     //强投机牌
//     {
//         {ACTION_FOLD, ACTION_FOLD, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_CALL,  ACTION_CALL,  ACTION_CHECK},
//         {ACTION_CALL, ACTION_CALL, ACTION_CALL,  ACTION_CALL,  ACTION_CHECK},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_CALL},
//         {ACTION_CALL, ACTION_CALL, ACTION_CALL,  ACTION_CALL,  ACTION_CALL},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
//     },
// 
//     //混合牌
//     {
//         {ACTION_FOLD, ACTION_FOLD, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_CALL,  ACTION_CALL},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_CALL,  ACTION_CALL,  ACTION_CALL},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_CHECK},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_CHECK},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
//     },
// 
//     //偷注牌
//     {
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_RAISE},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
//     },
// 
//     //丢弃牌
//     {
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_CALL},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
//         {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
//     },
// };

static Action s_hold_card_action_tab[CARD_STRENGTH_COUNT][PLAYER_ACTION_COUNT][MY_POSITION_COUNT] = 
{
    //超强牌
    {   //靠前         //居中        //靠后        //小盲注      //大盲注
        {ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
        {ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
        {ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
        {ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
        {ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
        {ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE, ACTION_RAISE},
    },

    //强牌
    {
        {ACTION_CALL, ACTION_CALL, ACTION_CALL, ACTION_CALL, ACTION_RAISE},
        {ACTION_CALL, ACTION_CALL, ACTION_CALL, ACTION_CALL, ACTION_CALL},
        {ACTION_CALL, ACTION_CALL, ACTION_CALL, ACTION_CALL, ACTION_CALL},
        {ACTION_FOLD,  ACTION_CALL, ACTION_CALL, ACTION_CALL, ACTION_CALL},
        {ACTION_CALL,  ACTION_CALL,  ACTION_CALL,  ACTION_CALL,  ACTION_CALL},
        {ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
        },

        //中等牌
        {
            {ACTION_FOLD, ACTION_CALL, ACTION_CALL, ACTION_CALL, ACTION_RAISE},
            {ACTION_FOLD, ACTION_CALL, ACTION_CALL, ACTION_CALL, ACTION_CALL},
            {ACTION_FOLD, ACTION_CALL, ACTION_CALL, ACTION_CALL, ACTION_CALL},
            {ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD,  ACTION_CALL},
            {ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD,  ACTION_CALL}, //TODO:KQs跟注
            {ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
        },

        //强投机牌
        {
            {ACTION_FOLD, ACTION_FOLD, ACTION_CALL, ACTION_CALL, ACTION_RAISE},
            {ACTION_FOLD, ACTION_FOLD, ACTION_CALL,  ACTION_CALL,  ACTION_CHECK},
            {ACTION_CALL, ACTION_CALL, ACTION_CALL,  ACTION_CALL,  ACTION_CHECK},
            {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_CALL},
            {ACTION_CALL, ACTION_CALL, ACTION_CALL,  ACTION_CALL,  ACTION_CALL},
            {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
            },

            //混合牌
            {
                {ACTION_FOLD, ACTION_FOLD, ACTION_CALL, ACTION_CALL, ACTION_RAISE},
                {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_CALL,  ACTION_CALL},
                {ACTION_FOLD, ACTION_FOLD, ACTION_CALL,  ACTION_CALL,  ACTION_CALL},
                {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_CHECK},
                {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_CHECK},
                {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
            },

            //偷注牌
            {
                {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_RAISE},
                {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
                {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
                {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
                {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
                {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
                },

                //丢弃牌
                {
                    {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_CALL},
                    {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
                    {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
                    {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
                    {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
                    {ACTION_FOLD, ACTION_FOLD, ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},
                },
};

//TODO:在你之前有超过ici加注(Raise)的情况,除了持有很强的牌,即AA,KK,QQ/AKs/AKo可以继续加注以外,其他的牌面都应弃牌
static CARD_STRENGTH GetCardStrength()
{
    //AA,KK,QQ,AKs,AKo
    if (   ((g_holdcard[0].point == "A") && (g_holdcard[1].point == "A"))
        || ((g_holdcard[0].point == "K") && (g_holdcard[1].point == "K")) 
        || ((g_holdcard[0].point == "Q") && (g_holdcard[1].point == "Q"))
        || ((g_holdcard[0].point == "A") && (g_holdcard[1].point == "K"))
        || ((g_holdcard[0].point == "K") && (g_holdcard[1].point == "A")))
    {
        return VERY_STRENGTH;
    }

    //JJ, 10-10, 99 / AQs, AQo, AJs 
    if (   ((g_holdcard[0].point == "J") && (g_holdcard[1].point == "J"))
        || ((g_holdcard[0].point == "10") && (g_holdcard[1].point == "10")) 
        || ((g_holdcard[0].point == "9") && (g_holdcard[1].point == "9"))
        || ((g_holdcard[0].point == "A") && (g_holdcard[1].point == "Q"))
        || ((g_holdcard[0].point == "Q") && (g_holdcard[1].point == "A"))
        || ((g_holdcard[0].point == "A") && (g_holdcard[1].point == "J") && (g_holdcard[0].color == g_holdcard[1].color))
        || ((g_holdcard[0].point == "J") && (g_holdcard[1].point == "A") && (g_holdcard[0].color == g_holdcard[1].color)))
    {
        return STRENGTH;
    }

    // AJo, A-10s, A-10o, KQs, KQo
    if (   ((g_holdcard[0].point == "A") && (g_holdcard[1].point == "J"))
        || ((g_holdcard[0].point == "J") && (g_holdcard[1].point == "A"))
        || ((g_holdcard[0].point == "A") && (g_holdcard[1].point == "10"))
        || ((g_holdcard[0].point == "10") && (g_holdcard[1].point == "A"))
        || ((g_holdcard[0].point == "K") && (g_holdcard[1].point == "Q"))
        || ((g_holdcard[0].point == "Q") && (g_holdcard[1].point == "K")) )
    {
        return NORMAL;
    }

    // 从88 到 22 /KJs, K-10s, QJs, Q-10s, J-10s, 10-9s 
    if (   ((g_holdcard[0].point == g_holdcard[1].point))
        || ((g_holdcard[0].point == "K") && (g_holdcard[1].point == "J") && (g_holdcard[0].color == g_holdcard[1].color)) 
        || ((g_holdcard[0].point == "K") && (g_holdcard[1].point == "10") && (g_holdcard[0].color == g_holdcard[1].color))
        || ((g_holdcard[0].point == "Q") && (g_holdcard[1].point == "J") && (g_holdcard[0].color == g_holdcard[1].color))
        || ((g_holdcard[0].point == "Q") && (g_holdcard[1].point == "10") && (g_holdcard[0].color == g_holdcard[1].color))
        || ((g_holdcard[0].point == "J") && (g_holdcard[1].point == "10") && (g_holdcard[0].color == g_holdcard[1].color))
        || ((g_holdcard[0].point == "10") && (g_holdcard[1].point == "9") && (g_holdcard[0].color == g_holdcard[1].color))
        || ((g_holdcard[0].point == "J") && (g_holdcard[1].point == "K") && (g_holdcard[0].color == g_holdcard[1].color)) 
        || ((g_holdcard[0].point == "10") && (g_holdcard[1].point == "K") && (g_holdcard[0].color == g_holdcard[1].color))
        || ((g_holdcard[0].point == "J") && (g_holdcard[1].point == "Q") && (g_holdcard[0].color == g_holdcard[1].color))
        || ((g_holdcard[0].point == "10") && (g_holdcard[1].point == "Q") && (g_holdcard[0].color == g_holdcard[1].color))
        || ((g_holdcard[0].point == "10") && (g_holdcard[1].point == "J") && (g_holdcard[0].color == g_holdcard[1].color))
        || ((g_holdcard[0].point == "9") && (g_holdcard[1].point == "10") && (g_holdcard[0].color == g_holdcard[1].color)) )
    {
        return SPECULARION;
    }

    if (   
           ((g_holdcard[0].point == "K") && (g_holdcard[1].point == "J"))
        || ((g_holdcard[0].point == "K") && (g_holdcard[1].point == "10"))
        || ((g_holdcard[0].point == "Q") && (g_holdcard[1].point == "J"))
        || ((g_holdcard[0].point == "Q") && (g_holdcard[1].point == "10"))
        || ((g_holdcard[0].point == "J") && (g_holdcard[1].point == "10"))
        || ((g_holdcard[0].point == "A") && (g_holdcard[0].color == g_holdcard[1].color)) 
        || ((g_holdcard[0].point == "K") && (g_holdcard[1].point == "9") && (g_holdcard[0].color == g_holdcard[1].color))
        || ((g_holdcard[0].point == "9") && (g_holdcard[1].point == "8") && (g_holdcard[0].color == g_holdcard[1].color))
        || ((g_holdcard[0].point == "8") && (g_holdcard[1].point == "7") && (g_holdcard[0].color == g_holdcard[1].color))     
        || ((g_holdcard[0].point == "J") && (g_holdcard[1].point == "K"))
        || ((g_holdcard[0].point == "10") && (g_holdcard[1].point == "K"))
        || ((g_holdcard[0].point == "J") && (g_holdcard[1].point == "Q"))
        || ((g_holdcard[0].point == "10") && (g_holdcard[1].point == "Q"))
        || ((g_holdcard[0].point == "10") && (g_holdcard[1].point == "J"))
        || ((g_holdcard[1].point == "A") && (g_holdcard[0].color == g_holdcard[1].color)) 
        || ((g_holdcard[0].point == "9") && (g_holdcard[1].point == "K") && (g_holdcard[0].color == g_holdcard[1].color))
        || ((g_holdcard[0].point == "8") && (g_holdcard[1].point == "9") && (g_holdcard[0].color == g_holdcard[1].color))
        || ((g_holdcard[0].point == "7") && (g_holdcard[1].point == "8") && (g_holdcard[0].color == g_holdcard[1].color)) )
    {
        return MIXED;
    }

    //AA,KK,QQ,AKs,AKo
    if (   ((g_holdcard[0].point == "A") && ((g_holdcard[1].point == "9") || (g_holdcard[1].point == "8") || (g_holdcard[1].point == "7")))
        || ((g_holdcard[0].point == "K") && ((g_holdcard[1].point == "9") || (g_holdcard[1].point == "8") || (g_holdcard[1].point == "7"))) 
        || ((g_holdcard[0].point == "Q") && ((g_holdcard[1].point == "9") || (g_holdcard[1].point == "8") || (g_holdcard[1].point == "7") ))
        || ((g_holdcard[1].point == "A") && ((g_holdcard[0].point == "9") || (g_holdcard[0].point == "8") || (g_holdcard[1].point == "7")))
        || ((g_holdcard[1].point == "K") && ((g_holdcard[0].point == "9") || (g_holdcard[0].point == "8") || (g_holdcard[1].point == "7")))
        || ((g_holdcard[1].point == "Q") && ((g_holdcard[0].point == "9") || (g_holdcard[0].point == "8") || (g_holdcard[1].point == "7"))) )
    {
        return STEAL_CARD;
    }

    return FOLD_CARD;
}

typedef enum
{
    HAVE_A = 0,
    HAVE_K_OVER_8 = 1,
    HAVE_Q_OVER_8 = 2,
    HAVE_J_OVER_8 = 3,
    HAVE_PAIR = 4,
    OTHER_CARD = 5,
    CARD_COUNT
}LESS_PLAYER_CARD_TYPE;

const static int k_less_players = 3;

//0 1 2 -> 1,2,3
Action less_players_tab[CARD_COUNT][k_less_players] = 
{
    //1个对手      2个对手       3个对手
    {ACTION_CHECK, ACTION_CHECK, ACTION_CHECK}, // 有A
    {ACTION_CHECK, ACTION_CHECK, ACTION_FOLD},  // 有K,并且另外牌超过8
    {ACTION_CHECK, ACTION_CHECK, ACTION_FOLD},  // 有Q,并且另外牌超过8
    {ACTION_CHECK, ACTION_FOLD,  ACTION_FOLD},  // 有J,并且另外牌超过8
    {ACTION_CHECK, ACTION_CHECK, ACTION_CHECK}, // 有对
    {ACTION_FOLD,  ACTION_FOLD,  ACTION_FOLD},  // 其他牌
};

bool IsBiggerThan8(const std::string& point)
{
    return (point == "A") || (point == "K") || (point == "Q") || (point == "J") || (point == "10") || (point == "9") || (point == "8");
}

//是小对子返回true
bool IsLessPair()
{
    if (g_holdcard[0].point != g_holdcard[1].point)
    {
        return false;
    }

    if (IsBiggerThan8(g_holdcard[0].point))
    {
        return false;
    }

    return true;
}

LESS_PLAYER_CARD_TYPE GetLessCardType()
{
    if ((g_holdcard[0].point == "A") || (g_holdcard[1].point == "A"))
    {
        return HAVE_A;
    }

    if (((g_holdcard[0].point == "K") && IsBiggerThan8(g_holdcard[1].point))
        || ((g_holdcard[1].point == "K") && IsBiggerThan8(g_holdcard[0].point)))
    {
        return HAVE_K_OVER_8;
    }

    if (((g_holdcard[0].point == "Q") && IsBiggerThan8(g_holdcard[1].point))
        || ((g_holdcard[1].point == "Q") && IsBiggerThan8(g_holdcard[0].point)))
    {
        return HAVE_Q_OVER_8;
    }

    if (((g_holdcard[0].point == "J") && IsBiggerThan8(g_holdcard[1].point))
        || ((g_holdcard[1].point == "J") && IsBiggerThan8(g_holdcard[0].point)))
    {
        return HAVE_J_OVER_8;
    }

    if (g_holdcard[0].point == g_holdcard[1].point)
    {
        return HAVE_PAIR;
    }

    return OTHER_CARD;
}

Action LessPlayersAction(int players)
{
    LESS_PLAYER_CARD_TYPE card_type = GetLessCardType();

    return less_players_tab[card_type][players - 1];  // 1,2,3 --> 0,1,2
}

void GetRaisePids(std::vector<int>& raise_pids)
{
    raise_pids.clear();

    const std::vector<InquirePlayerInfo>& player_inquare = gAllInquireMsgInfo[gCurBetRound].back();

    for (std::vector<InquirePlayerInfo>::const_iterator iter = player_inquare.begin(); 
        iter != player_inquare.end();
        ++iter)
    {
        if (iter->pid == GetPlayerId())
        {
            continue;
        }

        if ((iter->action != "fold") && (iter->action != "blind"))
        {
            raise_pids.push_back(iter->pid);
        }
    }
}

StrategyInof GetHoldCardAction()
{
    //第一轮action获取,基于基本表
    Action action = ACTION_FOLD;
    CARD_STRENGTH card_strength = GetCardStrength();
    PLAYER_ACTION player_action = GetPlayersAction();
    MY_POSITION postion = GetPlayerPosition();
    action = s_hold_card_action_tab[card_strength][player_action][postion];

    // 押注金额太大直接弃牌.
    int min_bet = GetMinBet();

    //2个玩家时,有些丢弃牌也要跟上
    if (ACTION_FOLD == action) 
    {
        if ((gAllPlayers.size() == 2) && (GetGoldGap() < 10000))
        {
            action = LessPlayersAction(gAllPlayers.size() - 1);
        }
    }
    
    //如果剩余筹码等于已经下注筹码,表示所有筹码已经进入.可以All in
    if (GetRemainBet() == GetTotalBet())
    {
        action = ACTION_CHECK;
        LOG("no jetton!\n");
    }
    
    if (MustWin())
    {
        action = ACTION_FOLD;
    }

    if (ACTION_FOLD != action)
    {
        if ((GetTotalBet() + min_bet) / (GetTotalBet() + GetRemainBet()) > 0.5)
        {
            action = ACTION_CHECK;
            LOG("Jetton Protected!\n");
        }
    }

    //应该丢弃的时候,发现有免费看牌机会,则要看牌.
    if (ACTION_FOLD == action)
    {
        if (0 == min_bet)
        {
            action = ACTION_CHECK;
            LOG("min_bet = 0.\n");
        }
    }

    LOG("Hold result :%d. %d.\n", action, min_bet);
    StrategyInof strategy;
    strategy.action = action;
    strategy.money = min_bet;

    return strategy;
}