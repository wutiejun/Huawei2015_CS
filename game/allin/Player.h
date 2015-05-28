#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>

#define TRACE(format, args...) \
    do \
    {\
        printf("%s:%d:", __FILE__, __LINE__);\
        printf(format, ## args);\
    }while(0)

/*****************************************************************************************
游戏流程:
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
******************************************************************************************/

/*****************************************************************************************
卡片出现的概率:
Poker Hand      牌的组合        理论出现概率        测试出现概率(3个玩家)
Royal Flush     皇家同花顺      649739:1            270:0
Straight Flush  同花顺          64973:1             270:0
Four-of-a-Kind  四条            4164:1              270:2
Full House      葫芦            693:1               270:12
Flush           同花色          508:1               270:23
------------------------------------------------------------------------------------------
Straight        顺子            254:1               270:49
Three-of-a-Kind 三条            46:1                270:48
TwoPair         二对            20:1                270:181
OnePair         一对            1.25:1              270:334
NoPair          没对子          1.002:1             270:158

超级同花顺  > 同花顺         > 四条           > 葫芦       > 同花  > 顺子     > 三条            > 两对    > 一对    > 高牌
Royal_Flush > Straight_Flush > Four_Of_A_Kind > Full_House > Flush > Straight > Three_Of_A_Kind > TwoPair > OnePair > High

策略: 加注 > 弃牌 > 跟注(过牌)

*****************************************************************************************/

/* 牌的组合类型 */
typedef enum CARD_TYPES_
{
    CARD_TYPES_None = 0,
    CARD_TYPES_High,
    CARD_TYPES_OnePair,
    CARD_TYPES_TwoPair,
    CARD_TYPES_Three_Of_A_Kind,
    CARD_TYPES_Straight,
    CARD_TYPES_Flush,
    CARD_TYPES_Full_House,
    CARD_TYPES_Four_Of_A_Kind,
    CARD_TYPES_Straight_Flush,
    CARD_TYPES_Royal_Flush,
} CARD_TYPES;

/* 根据游戏流程定义的各种消息类型 */
typedef enum SER_MSG_TYPES_
{
    SER_MSG_TYPE_none = 0,
    // 服务器到player的消息
    SER_MSG_TYPE_seat_info,
    SER_MSG_TYPE_blind,
    SER_MSG_TYPE_hold_cards,
    SER_MSG_TYPE_inquire,
    //
    SER_MSG_TYPE_flop,
    SER_MSG_TYPE_turn,
    SER_MSG_TYPE_river,
    SER_MSG_TYPE_showdown,
    SER_MSG_TYPE_pot_win,
    SER_MSG_TYPE_notify,
    SER_MSG_TYPE_game_over,
    //
} SER_MSG_TYPES;

/* 抽象消息结构 */
typedef struct GAME_MSG_
{
    SER_MSG_TYPES Type;
    char MsgRawData[1024];      /* 从日志中暂时没看到大于1024的消息 */
} GAME_MSG;

typedef struct GAME_MSG_reg_
{
    const char * MsgName;
    //
    int PlayerID;
    char PlayerName[32];
} GAME_MSG_reg;

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
    CARD_POINT_K,
    CARD_POINT_A,
} CARD_POIN;

/* play card */
typedef struct CARD_
{
    CARD_POIN Point;
    CARD_COLOR Color;
} CARD;

typedef enum CLIENT_MSG_TYPES_
{
    // player到服务器的消息
    CLIENT_MSG_TYPES_Reg,
    CLIENT_MSG_TYPES_ACT_check,
    CLIENT_MSG_TYPES_ACT_call,
    CLIENT_MSG_TYPES_ACT_raise,
    CLIENT_MSG_TYPES_ACT_all_in,
    CLIENT_MSG_TYPES_ACT_fold,
} CLIENT_MSG_TYPES;

/* 玩家的处理策略 */
typedef enum PLAYER_Action_
{
    ACTION_check,       /* 让牌，即在前面的玩家，什么也不做，把机会给后面的玩家 */
    ACTION_call,        /* 跟进，即前面有人raise，即不re-raise，也不弃牌，则call， */
    ACTION_allin,
    ACTION_raise,
    ACTION_fold,
} PLAYER_Action;

typedef enum PLAYER_SEAT_TYPES_
{
    // 一局中player类型
    PLAYER_SEAT_TYPES_none,
    PLAYER_SEAT_TYPES_button,
    PLAYER_SEAT_TYPES_small_blind,
    PLAYER_SEAT_TYPES_big_blind,
} PLAYER_SEAT_TYPES;

/* 玩家信息 */
typedef struct PLAYER_
{
    /* 静态属性 */
    char PlayerID[32];
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


/****************************************************************************************/
SER_MSG_TYPES Msg_GetMsgType(const char * pMsg, int MaxLen);

const char * Msg_GetMsgNameByType(SER_MSG_TYPES Type);

/****************************************************************************************/

#endif

