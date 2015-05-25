#pragma once
#include "tx_poker.h"
#include <string>
#include "CalcCardRate.h"
#include <vector>
#include <map>
//#include "log.h"

#define LOG(...)

typedef struct _Play_Round_Cards
{
    CardMask hand_cards;
    CardMask common_cards;
    CardMask turn_card;
    CardMask river_card;
}Play_Round_Cards;

typedef enum 
{
    PLAYER_IS_NORMAL = 0,
    PLAYER_IS_BUTTON,
    PLAYER_IS_SMALL_BLIND,
    PLAYER_IS_BIG_BLIND,
}PLAYER_TYPE;

typedef struct _Player 
{
    PLAYER_TYPE type; //玩家类型
    int pid;
    unsigned int money;
    std::string jetton; //筹码
}Player;


typedef enum _Action
{
    ACTION_BLIND,
    ACTION_CHECK,
    ACTION_CALL,
    ACTION_RAISE,
    ACTION_ALL_IN,
    ACTION_FOLD
}Action;

extern Play_Round_Cards gPlayerCards;
extern std::vector<Player> gAllPlayers;

extern uint32 gCurPotTotalbet;
extern int g_small_blind_bet;
extern int g_small_blind_pid;
extern int g_big_blind_bet;
extern int g_big_blind_pid;
extern int g_residual_turn;
extern int g_crruent_round;
extern int g_init_glod;
extern int g_cur_round_glod;

extern bool g_once_flag;

extern bool g_naughties_flag;

extern std::map<int,int> gPersonalityAnalyz;

typedef enum
{
    HOLD_BET_ROUND,      //底牌下注轮
    FLOP_BET_ROUND,      //公共牌下注轮
    TURN_BET_ROUND,      //转牌下注轮
    RIVER_BET_ROUND,     //河牌下注轮
    ALL_BET_ROUND_COUNT, //下注轮计数 --- 方便遍历
    NOT_BET_ROUND,       //非下注轮
}BET_ROUND;

extern BET_ROUND gCurBetRound;

typedef struct
{
    int pid;
    int jetton;
    int money;
    int bet;
    std::string action;
}InquirePlayerInfo;

//第一维为投注轮
//第二维单轮得到信息次数
//第三维所有玩家本轮投注信息
extern std::vector< std::vector< std::vector<InquirePlayerInfo> > >  gAllInquireMsgInfo;

extern std::map<int,int> gAllCurRoundBet;

typedef enum
{
    ALL_FOLD = 0,           //所有玩家弃牌
    ONE_CALL,               //一个玩家跟注
    MORE_CALL,              //两个以上玩家跟注
    ONE_RAISE_NO_CALL,      //只有一个玩家加注,但是没有玩家跟注
    ONE_RAISE_ONEMORE_CALL, //只有一个玩家加注,至少一个玩家跟注
    MORE_RAISE,             //超过一个玩家加注
    PLAYER_ACTION_COUNT
}PLAYER_ACTION;

typedef enum
{
    FRONT_POS = 0,   //前面位置
    CENTER_POS,      //中间位置
    BEHIND_POS,      //后面位置
    SMALL_BLIND_POS, //小盲注位置
    BIG_BLIND_POS,   //大盲注位置
    MY_POSITION_COUNT
}MY_POSITION;

#define RAND rand()/(RAND_MAX/100)

void InitGlobalData();
void ClearRoundGlobalData();

int CovertMsgPoint(const std::string &msg);
uint32 CovertMsgSuit(const std::string &msg);

void SavePlayer(PLAYER_TYPE type, int pid, uint32 money, std::string &jetton);

void CleanPlayer();

int GetMySelfJetton();
void CleanPlayer();

int GetMinBet();

PLAYER_ACTION GetPlayersAction();

MY_POSITION GetPlayerPosition();
int GetMySelfGlod();

int GetCurrentPlayerNum();

int GetMySelfBet();

PLAYER_ACTION GetPlayersAction();
double GetMadnessRate(std::vector<int> &pids);

int GetMyCurRoundPosition();
//本手牌共下注的筹码数
int GetTotalBet();

//自己的手中剩余筹码数
int GetRemainBet();

int GetWinJetton(int min_bet);

int GetPlayersTotalMoney(int pid);

int GetGoldGap();

bool MustWin();