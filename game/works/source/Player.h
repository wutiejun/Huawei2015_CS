
#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>

#define GAME_OVER 0xffee

void SetPlayerId(int id);

#define WSADESCRIPTION_LEN 128
#define WSASYSSTATUS_LEN 128

typedef struct WSAData_ {
   int wVersion;
   int wHighVersion;
   char szDescription[WSADESCRIPTION_LEN+1];
   char szSystemStatus[WSASYSSTATUS_LEN+1];
   unsigned short iMaxSockets;
   unsigned short iMaxUdpDg;
   char * lpVendorInfo;
} WSADATA;

/*若比赛结束, 应返回GAME_OVER*/
//int RoundEntry(int& socket);
int GameOver(void);
void GameStart(void);
int GetPlayerId(void);
int PlayerMsgProc(char* recv_buf);
int RoundEntry(int socket);

/*
1.	player向server注册自己的id和name（reg-msg） 
2.	while（还有2个及以上玩家并且未超过最大局数）
a)	发布座次信息：seat-info-msg（轮流坐庄）
b)	强制押盲注：blind-msg
c)	为每位牌手发两张底牌：hold-cards-msg
d)	翻牌前喊注： inquire-msg/action-msg（多次）
e)	发出三张公共牌：flop-msg 
f)	翻牌圈喊注：inquire-msg/action-msg（多次）
g)	发出一张公共牌（转牌）：turn-msg
h)	转牌圈喊注： inquire-msg/action-msg（多次）
i)	发出一张公共牌（河牌）：river-msg
j)	河牌圈喊注：inquire-msg/action-msg（多次）
k)	若有两家以上未盖牌则摊牌比大小：showdown-msg
l)	公布彩池分配结果：pot-win-msg
3.	本场比赛结束（game-over-msg）
*/
typedef enum MSG_TYPES_
{
    MSG_TYPE_none = 0,
    // 服务器到player的消息
    MSG_TYPE_reg,
    MSG_TYPE_seat_info,
    MSG_TYPE_blind,
    MSG_TYPE_hold_cards,
    MSG_TYPE_inquire,
    //
    MSG_TYPE_flop,
    MSG_TYPE_turn,
    MSG_TYPE_river,
    MSG_TYPE_showdown,
    MSG_TYPE_pot_win,
    MSG_TYPE_game_over,
    
    // player到服务器的消息
    MSG_TYPE_action_check,
    MSG_TYPE_action_call, 
    MSG_TYPE_action_raise,
    MSG_TYPE_action_all_in,
    MSG_TYPE_action_fold,
    //
} MSG_TYPES;

/* 抽象消息结构 */
typedef struct GAME_MSG_
{
    
} GAME_MSG;

/* 卡片颜色 */
typedef enum CARD_COLOR_
{
    CARD_COLOR_Unknow,          /* 用于标识对手的未知卡片 */
    CARD_COLOR_SPADES = 1,
    CARD_COLOR_HEARTS,
    CARD_COLOR_CLUBS,
    CARD_COLOR_DIAMONDS,
} CARD_COLOR;

/* 卡片大小 */
typedef enum CARD_POINT_
{
    CARD_POINT_Unknow,      /* 用于标识对手的未知卡片 */
    CARD_POINT_2 = 2,
    CARD_POINT_3,
    CARD_POINT_4, 
    CARD_POINT_5,
    CARD_POINT_6,
    CARD_POINT_7,
    CARD_POINT_8,
    CARD_POINT_9,
    CARD_POINT_10,
    CARD_POINT_J,
    CARD_POINT_Q,
    CARD_POINT_J,
    CARD_POINT_A,
} CARD_POIN;

/* play card */
typedef struct CARD_
{
    CARD_POIN Point;
    CARD_COLOR Color;
} CARD;

/* 玩家的处理策略 */
typedef struct PLAYER_Action_
{
    ACTION_check,       /* 让牌，即在前面的玩家，什么也不做，把机会给后面的玩家 */
    ACTION_call,        /* 跟进，即前面有人raise，即不re-raise，也不弃牌，则call， */
    
} PLAYER_Action;

/* 玩家信息 */
typedef struct PLAYER_
{
    /* 静态属性 */    
    int PlayerID;
    char PlayerName[32];    /* player name, length of not more than 20 bytes */

    /* 动态player 信息 */    
    int Status;             /* 当前局的当前状态 */
    int SeatIndex;          /* 当前局的位置 */
    CARD HoldCards[2];      /* 选手的两张底牌，不确定的 */

} PLAYER;

/* 每一局信息 */
typedef struct RoundInfo_
{
    int PlayerCount;        /* 玩家个数 */
    PLAYER * Players[8];    /* 8个玩家，暂时用指针 */
    CARD FlopCards[3];      /* 3张公牌 */
    CARD TurnCards[1];      /* 1张转牌 */
    CARD RiverCards[1];     /* 1张河牌 */
} RoundInfo;

#endif

