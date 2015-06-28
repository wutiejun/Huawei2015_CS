#include <stdio.h>
#include <stdlib.h>  //
#include <string.h>  //strlen
#include <unistd.h>  //usleep/close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "pthread.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <errno.h>
#include "Player.h"

/* 负责处理策略 */

obj_queue g_round_queue;

void QueueInit(obj_queue * pQueue)
{
    memset(pQueue, 0, sizeof(obj_queue));
    pthread_mutex_init(&pQueue->Lock, NULL);
    return;
}

void QueueAdd(obj_queue * pQueue, void * pMsg, int size)
{
    queue_entry *pNewMsg = NULL;
    do
    {
        pNewMsg = (queue_entry *)malloc(sizeof(queue_entry) + size);
        if (pNewMsg == NULL)
        {
            TRACE("%lu %d\r\n", sizeof(queue_entry) + size, errno);
            return;
        }
    } while (pNewMsg == NULL);

    memset(pNewMsg, 0, sizeof(queue_entry) + size);

    pNewMsg->pObjData = pNewMsg + 1;
    pNewMsg->pNextMsg = NULL;
    memcpy(pNewMsg->pObjData, pMsg, size);

    pthread_mutex_lock(&pQueue->Lock);
    if (pQueue->MsgCount == 0)
    {
        /* 两个必然同时为空 */
        pQueue->pFirstMsg = pNewMsg;
    }
    else
    {
        pQueue->pLastMsg->pNextMsg = pNewMsg;
    }
    pQueue->pLastMsg = pNewMsg;
    pQueue->MsgCount ++;
    pthread_mutex_unlock(&pQueue->Lock);

    return;
}

queue_entry * QueueGet(obj_queue * pQueue)
{
    queue_entry *pMsg = NULL;
    pthread_mutex_lock(&pQueue->Lock);
    if (pQueue->MsgCount > 0)
    {
        pMsg = pQueue->pFirstMsg;
        pQueue->pFirstMsg = (queue_entry *)pQueue->pFirstMsg->pNextMsg;
        if (pQueue->pFirstMsg == NULL)
        {
            pQueue->pLastMsg = NULL;
        }
        pQueue->MsgCount --;
    }
    pthread_mutex_unlock(&pQueue->Lock);
    return pMsg;
}

/* 赢牌类型 */
typedef struct STG_WIN_CARDS_
{
    int ShowTimes;
    int WinTimes;
    //CARD_TYPES WinType;     /* 赢牌的类型 */
    //CARD_POINT MaxPoint;    /* 赢牌的最大点数 */
    //CARD_POINT SedPoint;    /* 赢牌的次大点数，这个是否需要考虑？可以不考虑，如果是两对的情况，次大牌可以再通过一对的情况来判断 */
} STG_WIN_CARDS;

typedef struct STG_DATA_
{
    int RunningFlag;

    pthread_mutex_t Lock;
    pthread_t subThread;

    /* 赢牌的数据，一张大的数据表，方便快速索引 */
    STG_WIN_CARDS AllWinCards[8][CARD_TYPES_Butt][CARD_POINTT_BUTT];
} STG_DATA;

STG_DATA g_stg = {0};

static void STG_Lock(void)
{
    pthread_mutex_lock(&g_stg.Lock);
}

static void STG_Unlock(void)
{
    pthread_mutex_unlock(&g_stg.Lock);
}

void STG_Debug_PrintWinCardData(void)
{
    int Type;
    int Point;
    int UserNum;
    for (UserNum = 1; UserNum < 8; UserNum ++)
    {
        TRACE("Player num:%d\r\n", UserNum);
        for (Type =  CARD_TYPES_High; Type < CARD_TYPES_Butt; Type ++)
        {
            for (Point = CARD_POINTT_2; Point < CARD_POINTT_BUTT; Point ++)
            {
                TRACE("Type:%s Max: %s Show:%5d; Win:%5d;\r\n",
                       Msg_GetCardTypeName((CARD_TYPES)Type),
                       GetCardPointName((CARD_POINT)Point),
                       g_stg.AllWinCards[UserNum][Type][Point].ShowTimes,
                       g_stg.AllWinCards[UserNum][Type][Point].WinTimes);
            }
        }
    }
}

/* 载入版型数据库 */
void STG_LoadStudyData(void)
{
    int fid = -1;
    int read_size = 0;

    memset(&g_stg.AllWinCards, 0, sizeof(g_stg.AllWinCards));

    fid = open("db.bin", O_RDONLY);
    if (fid == -1)
    {
        TRACE("Load db.bin error.\r\n");
        return;
    }

    read_size = read(fid, &g_stg.AllWinCards, sizeof(g_stg.AllWinCards));
    if (read_size != sizeof(g_stg.AllWinCards))
    {
        memset(&g_stg.AllWinCards, 0, sizeof(g_stg.AllWinCards));
        TRACE("Load db.bin error.\r\n");
    }
    close(fid);

    STG_Debug_PrintWinCardData();

    return;
}

/* 保存学习到的牌型数据 */
void STG_SaveStudyData(void)
{
    int fid = -1;
    fid = open("db.bin", O_CREAT|O_WRONLY|O_TRUNC|O_SYNC);
    if (fid == -1)
    {
        TRACE("Save db.bin error.\r\n");
        return;
    }
    write(fid, &g_stg.AllWinCards, sizeof(g_stg.AllWinCards));
    close(fid);
    TRACE("Save db.bin\r\n");
    STG_Debug_PrintWinCardData();
    return;
}

/* 保存一局的数据 */
void STG_SaveRoundData(RoundInfo * pRound)
{
    static int RoundIndex = 0;

    printf("====save round %d =========\r\n", pRound->RoundIndex);

    QueueAdd(&g_round_queue, pRound, sizeof(RoundInfo));

    memset(pRound, 0, sizeof(RoundInfo));
    //
    RoundIndex ++;
    pRound->RoundIndex = RoundIndex;

    return;
}

/* 查询全局数据库，取得指定牌型和最大点数的胜率  */
int STG_CheckWinRation(CARD_TYPES Type, CARD_POINT MaxPoint, int PlayerNum)
{
    //printf("STG_CheckWinRation:%d %d %d;\r\n", PlayerNum, (int)Type, (int)MaxPoint);
    int WinNum = g_stg.AllWinCards[PlayerNum - 1][Type][MaxPoint].WinTimes;
    int ShowNum = g_stg.AllWinCards[PlayerNum - 1][Type][MaxPoint].ShowTimes;

//    printf("Check Win Ration[%d]: %s %s: %d / %d;\r\n",
//           PlayerNum,
//          Msg_GetCardTypeName(Type),
//          GetCardPointName(MaxPoint),
//          WinNum, ShowNum);

    if (ShowNum > 0)
    {
        return (WinNum * 100) / ShowNum;
    }
    return 0;
}

/* 仅通过牌型来查询胜率，在不同场景下有比较好的优势，如解决一些机器人 */
int STG_CheckWinRationByType(CARD_TYPES Type, int PlayerNum)
{
    int index = 0;
    int WinNum = 0;
    int ShowNum  = 0;

    /* 读取数据，不用上锁 */
    for (index = CARD_POINTT_2; index < CARD_POINTT_BUTT; index ++)
    {
        WinNum += g_stg.AllWinCards[PlayerNum - 1][Type][index].WinTimes;
        ShowNum += g_stg.AllWinCards[PlayerNum - 1][Type][index].ShowTimes;
    }
    if (ShowNum > 0)
    {
        return (WinNum * 100) / ShowNum;
    }
    return 0;
}

/* 判断两张手牌是否是一对 */
bool STG_IsHoldPair(RoundInfo * pRound)
{
    return pRound->HoldCards.Cards[0].Point == pRound->HoldCards.Cards[1].Point;
}

/* 判断两张手牌是否是同花色 */
bool STG_IsHoldSameColor(RoundInfo * pRound)
{
    return pRound->HoldCards.Cards[0].Color == pRound->HoldCards.Cards[1].Color;
}

/* 取得本局还在场的完家 */
int STG_GetActivePlayer(RoundInfo * pRound)
{
    //pRound->Inquires.PlayerActions

}

/* 处理手牌阶段的inquire */
PLAYER_Action STG_GetHoldAction(RoundInfo * pRound)
{
    int WinRation = 0;
    CARD AllCards[2];
    CARD_POINT MaxPoint[CARD_TYPES_Butt];// = {(CARD_POINT)0};
    CARD_TYPES Type = CARD_TYPES_None;
    AllCards[0] = pRound->HoldCards.Cards[0];
    AllCards[1] = pRound->HoldCards.Cards[1];

    /* 先取两张牌型的胜率情况 */
    Type = STG_GetCardTypes(AllCards, 2, MaxPoint);
    WinRation = STG_CheckWinRation(Type, MaxPoint[Type], pRound->SeatInfo.PlayerNum);
    if (WinRation > 50)
    {
        return ACTION_raise;
    }

    /* 再分析当前对手情况 */


    /*  */

    return ACTION_check;
}

/* 根据给定的两红手牌和点数，看最大点数的牌是否在自己手里 */
bool STG_IsMaxPointInHand(CARD_POINT MaxPoint, CARD Cards[2])
{
    if (   (Cards[0].Point == MaxPoint)
        || (Cards[1].Point == MaxPoint))
    {
        return true;
    }
    return false;
}

/*
    河牌时的处理，关键
*/
PLAYER_Action STG_GetRiverAction(RoundInfo * pRound)
{
    int WinRation = 0;
    CARD AllCards[7];
    CARD_POINT MaxPoint[CARD_TYPES_Butt];// = {(CARD_POINT)0};
    CARD_TYPES Type = CARD_TYPES_None;
    bool IsMaxInHand = false;
    AllCards[0] = pRound->PublicCards.Cards[0];
    AllCards[1] = pRound->PublicCards.Cards[1];
    AllCards[2] = pRound->PublicCards.Cards[2];
    AllCards[3] = pRound->PublicCards.Cards[3];
    AllCards[4] = pRound->PublicCards.Cards[4];
    AllCards[5] = pRound->HoldCards.Cards[0];
    AllCards[6] = pRound->HoldCards.Cards[1];
    Type = STG_GetCardTypes(AllCards, 7, MaxPoint);
    WinRation = STG_CheckWinRation(Type, MaxPoint[Type], pRound->SeatInfo.PlayerNum);

    bool IsMaxInMyHand = STG_IsMaxPointInHand(MaxPoint[Type], pRound->HoldCards.Cards);
//    printf("Total pot on ground:%d win:%d ismax:%d;\r\n",
//           pRound->Inquires[3].TotalPot,
//           WinRation, IsMaxInMyHand);
    if (IsMaxInMyHand)
    {
        if (WinRation <= 30)
        {
    //        printf("Round %d allin:%d; \r\n", pRound->RoundIndex, WinRation,
    //               Msg_GetCardTypeName(Type),
    //               GetCardPointName(MaxPoint[Type]));
            //pRound-> 记录raise的次数和总金额，如果超过比例，就不再raise，而是check或者fold
            if (pRound->Inquires[3].TotalPot > 1000)
            {
                return ACTION_fold;
            }
            else
            {
                return ACTION_check;
            }
        }
        else if (WinRation <= 50)
        {
            if (pRound->RaiseTimes ++ < 2)
            {
                return ACTION_raise;    /* 只riase到指定比例 */
            }
            return ACTION_check;
        }
        else if (WinRation <= 70)
        {
            if (pRound->RaiseTimes ++ < 3)
            {
                return ACTION_raise;    /* 只riase到指定比例 */
            }
            return ACTION_check;
        }
        else
        {
            return ACTION_allin;    /*  */
        }
    }
    return ACTION_fold;
}

#define INIT_TWOCARD_NUM 40
#define INIT_TWOCARD_NUM_EXTEND 38

typedef struct
{
	int cardinfo;  //两张牌，如AA表示成1414，KK表示成13130,个位表示花色 0：不同花色，1：同花色
	int No_Pet;    //在你之前无人加注
	int With_Pet;  //在你之前有人加注
}INIT_TWOCARD_INFO_EXTEND;


typedef enum
{
	TWOCARD_INFO_R, // 无论你前面玩家如何下注，你都要加注
	TWOCARD_INFO_C, //无论有多少个玩家进入游戏，你都要跟注
	TWOCARD_INFO_C_RFI_LP, //无论有多少个玩家进入游戏，你都要跟注 处于后位 加注
	TWOCARD_INFO_C1, //在你之前至少有一个玩家跟注，你才可以跟注，否则弃牌
	TWOCARD_INFO_C2, //在你之前如果有两个或两个以上的玩家进入游戏，你才可以跟注，否则弃牌
	TWOCARD_INFO_C2_RFI_LP, //在你之前如果有两个或两个以上的玩家进入游戏，你才可以跟注，否则弃牌 处于后位加注
	TWOCARD_INFO_C3, //在你之前如果有三个或三个以上的玩家进入游戏，你才可以跟注，否则弃牌
	TWOCARD_INFO_C3_RFI_LP, //在你之前如果有三个或三个以上的玩家进入游戏，你才可以跟注，否则弃牌 处于后位加注
	TWOCARD_INFO_C4, //在你之前如果有四个或四个以上的玩家进入游戏，你才可以跟注，否则弃牌
	TWOCARD_INFO_RR, // 你应该再加注
	TWOCARD_INFO_F, // 你应该弃牌

	TWOCARD_INFO_BUTT
}TWOCARD_INFO_BEHAIVOR;

INIT_TWOCARD_INFO_EXTEND Init_TwoCard_Info_Extend[INIT_TWOCARD_NUM_EXTEND] =
{
    {14140,TWOCARD_INFO_R,TWOCARD_INFO_RR},
    {13130,TWOCARD_INFO_R,TWOCARD_INFO_RR},
    {12120,TWOCARD_INFO_R,TWOCARD_INFO_RR},
    {14131,TWOCARD_INFO_R,TWOCARD_INFO_RR},
    {11110,TWOCARD_INFO_R,TWOCARD_INFO_C},
    {10100,TWOCARD_INFO_R,TWOCARD_INFO_C},
    {14130,TWOCARD_INFO_R,TWOCARD_INFO_C},
    {14121,TWOCARD_INFO_R,TWOCARD_INFO_C},
    {9090, TWOCARD_INFO_C,TWOCARD_INFO_C2},
    {14120,TWOCARD_INFO_C,TWOCARD_INFO_C2},
    {14111,TWOCARD_INFO_C,TWOCARD_INFO_C2},
    {13121,TWOCARD_INFO_C,TWOCARD_INFO_C2},
    {13120,TWOCARD_INFO_C,TWOCARD_INFO_F},
    {8080, TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {7070, TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {6060, TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {5050, TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {4040, TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {3030, TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {2020, TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {14021,TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {14031,TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {14041,TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {14051,TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {14061,TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {14071,TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {14081,TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {14091,TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {14101,TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {13111,TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {13101,TWOCARD_INFO_C3,TWOCARD_INFO_F},
    {12111,TWOCARD_INFO_C2,TWOCARD_INFO_C4},
    {12101,TWOCARD_INFO_C3,TWOCARD_INFO_F},
    {11101,TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {10091,TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {9081,TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {8071,TWOCARD_INFO_C3,TWOCARD_INFO_C4},
    {7061,TWOCARD_INFO_C3,TWOCARD_INFO_C4},
};


MSG_INQUIRE_INFO * Msg_GetCurrentInquireInfo(RoundInfo * pRound);

//获取首轮在我之前有多少存活玩家数
int ST_GetAlivePlayerNum(RoundInfo *pRound)
{
    int cnt = 0;
    MSG_INQUIRE_INFO *pInquireInfo = Msg_GetCurrentInquireInfo(pRound);

	for(unsigned int i = 0; i < pInquireInfo->PlayerNum; i++)
	{
		if (pInquireInfo->PlayerActions[i].Action != ACTION_fold)
		{
			cnt++;
		}
	}

	return cnt;
}
//获取首轮在我之前有多少加注玩家数
int ST_GetAlivePetPlayerNum(RoundInfo *pRound)
{
    int cnt = 0;
    MSG_INQUIRE_INFO *pInquireInfo = Msg_GetCurrentInquireInfo(pRound);

	for(unsigned int i = 0; i < pInquireInfo->PlayerNum; i++)
	{
		if (pInquireInfo->PlayerActions[i].Action == ACTION_raise)
		{
			cnt++;
		}
	}

	return cnt;
}

/*当前存活的玩家数小于等于3，且是2张牌时的策略 */
/************************************************************************/
/* 功能：初始牌通过预知条件判断                                        */
/* 输入：初始牌                                                       */
/* 输出：                                                           */
/* 输出：每局开始时调用                                                 */
/************************************************************************/
PLAYER_Action Get_Init_Two_Card_Action_Extend(RoundInfo *pRound)
{
	unsigned int i;
	int cardinfo;
	int Max,Min;
	CARD CardInfo[2];
	int type = 0;
	int alive_num;
	int alive_pet_num;
	int No_Pet_Action = TWOCARD_INFO_BUTT;    //在之前无人加注
	int With_Pet_Action = TWOCARD_INFO_BUTT;  //在之前有人加注

    /* 取得两张手牌 */
	memcpy(pRound->HoldCards.Cards, CardInfo, sizeof(CardInfo));

	if (CardInfo[0].Point > CardInfo[1].Point)
	{
		Max = CardInfo[0].Point;
		Min = CardInfo[1].Point;
	}
	else
	{
		Max = CardInfo[1].Point;
		Min = CardInfo[0].Point;
	}
	if (CardInfo[0].Color == CardInfo[1].Color)
	{
		type = 1;
	}
	cardinfo = Max*1000 + Min*10 + type;

	for (i = 0;i<INIT_TWOCARD_NUM_EXTEND;i++)
	{
		if (Init_TwoCard_Info_Extend[i].cardinfo == cardinfo)
		{
			No_Pet_Action = Init_TwoCard_Info_Extend[i].No_Pet;
			With_Pet_Action = Init_TwoCard_Info_Extend[i].With_Pet;
			break;
		}
	}

	alive_num = ST_GetAlivePlayerNum(pRound);
	alive_pet_num = ST_GetAlivePetPlayerNum(pRound);

	if (alive_pet_num == 0)
	{
		/*无论你前面玩家如何下注，你都要加注*/
		if (No_Pet_Action == TWOCARD_INFO_R)
		{
			return ACTION_raise;
		}
		/*无论有多少个玩家进入游戏，你都要跟注*/
		else if(No_Pet_Action == TWOCARD_INFO_C)
		{
			return ACTION_call;
		}
		/*在你之前如果有两个或两个以上的玩家进入游戏，你才可以跟注，否则弃牌*/
		else if(No_Pet_Action == TWOCARD_INFO_C2)
		{
			if (alive_num > 1)
			{
				return ACTION_call;
			}
			else
			{
				return ACTION_fold;
			}
		}
		/*在你之前如果有三个或三个以上的玩家进入游戏，你才可以跟注，否则弃牌*/
		else if(No_Pet_Action == TWOCARD_INFO_C3)
		{
			if (alive_num > 2)
			{
				return ACTION_call;
			}
			else
			{
				return ACTION_fold;
			}
		}
		/*在你之前如果有四个或四个以上的玩家进入游戏，你才可以跟注，否则弃牌*/
		else if(No_Pet_Action == TWOCARD_INFO_C4)
		{
			if (alive_num > 3)
			{
				return ACTION_call;
			}
			else
			{
				return ACTION_fold;
			}
		}
	}
	else
	{
		/*你应该再加注*/
		if (With_Pet_Action == TWOCARD_INFO_RR)
		{
			return ACTION_raise;
		}
		/*无论有多少个玩家进入游戏，你都要跟注*/
		else if(With_Pet_Action == TWOCARD_INFO_C)
		{
			return ACTION_call;
		}
		/*在你之前如果有两个或两个以上的玩家进入游戏，你才可以跟注，否则弃牌*/
		else if(With_Pet_Action == TWOCARD_INFO_C2)
		{
			if (alive_num > 1)
			{
				return ACTION_call;
			}
			else
			{
				return ACTION_fold;
			}
		}
		/*在你之前如果有四个或四个以上的玩家进入游戏，你才可以跟注，否则弃牌*/
		else if(With_Pet_Action == TWOCARD_INFO_C4)
		{
			if (alive_num > 3)
			{
				return ACTION_call;
			}
			else
			{
				return ACTION_fold;
			}
		}
		/*你应该弃牌*/
		else if(With_Pet_Action == TWOCARD_INFO_F)
		{
			return ACTION_fold;
		}
	}

	return ACTION_BUTTON;
}

/* 只会在inquire中读取处理动作 */
int STG_GetAction(RoundInfo * pRound, char ActionBuf[128])
{
    const char * pAction = NULL;
    PLAYER_Action  Action = ACTION_fold;
    Action = ACTION_check;

    //printf("Get action:%d;\r\n", pRound->RoundStatus);

    switch (pRound->RoundStatus)
    {
    default:
         break;
    case SER_MSG_TYPE_hold_cards_inquire:/* 只有两张手牌时的inqurie */
        //Action = ACTION_check;
        Action = Get_Init_Two_Card_Action_Extend(pRound);
        if (Action == ACTION_BUTTON)
        {
            Action = ACTION_fold;
        }
        break;
    case SER_MSG_TYPE_flop_inquire:     /* 三张公牌后的inqurie */
        //Action = ACTION_check;
        //Action = STG_GetFlopAction(pRound);
        break;
    case SER_MSG_TYPE_turn_inquire:     /* 四张公牌后的inqurie */
        //Action = ACTION_check;
        //Action = STG_GetTurnAction(pRound);
        break;
    case SER_MSG_TYPE_river_inquire:
        Action = STG_GetRiverAction(pRound);
        break;
    }

    pAction = GetActionName(Action);
    if (Action == ACTION_raise)
    {
        return sprintf(ActionBuf, "%s %d", pAction, 1);
    }
    else
    {
        return sprintf(ActionBuf, "%s", pAction);
    }
}

/* 处理inquire的回复 */
void STG_Inquire_Action(RoundInfo * pRound)
{
    //TRACE("Response check.\r\n");
    //const char* response = "check";
    char ActionBufer[128] = {0};
    int ResNum = STG_GetAction(pRound, ActionBufer);
    TRACE("Get Action:%s.\r\n", ActionBufer);
    ResponseAction(ActionBufer, ResNum);
    return;
}

/* 根据牌的大小排序 */
void STG_SortCardByPoint(CARD * Cards, int CardNum)
{
    int i = 0;
    int j = 0;
    CARD TempCard;

    for (i = 0; i < CardNum; i ++)
    {
        for (j = i + 1; j < CardNum; j ++)
        {
            if (Cards[j].Point < Cards[i].Point)
            {
                TempCard = Cards[i];
                Cards[i] = Cards[j];
                Cards[j] = TempCard;
            }
        }
    }
    return;
}

/* 判断两个手牌是否有相同的点数 */
bool STG_IsTwoHoldCardHasSamePoint(CARD CardsA[2], CARD CardsB[2])
{
    STG_SortCardByPoint(CardsA, 2);
    STG_SortCardByPoint(CardsB, 2);
    if (   (CardsA[0].Point == CardsB[0].Point)
        && (CardsA[1].Point == CardsB[1].Point))
    {
        return true;
    }
    return false;
}

static void print_card(CARD * pCard, int CardNum)
{
    int index = 0;
    printf("Card Info:\r\n");
    for (index = 0; index < CardNum; index ++)
    {
        printf("card_%d %s %s\n", index, GetCardColorName(pCard),
               GetCardPointName(pCard->Point));
        pCard ++;
    }
    return;
}

/* 判断是否是同花顺，以及皇家同花顺, pCards已经是 */
CARD_TYPES SET_GetStraightType(CARD FlushCards[8], int CardNum, CARD_POINT * pMaxPoint)
{
    int index = 0;
    int StriaghtNum = 1;
    int MaxIndex = 0;
    CARD_POINT PrePoint = CARD_POINTT_Unknow;

    for (index = 0; index < CardNum; index ++)
    {
        if (FlushCards[index].Point == CARD_POINTT_Unknow)
        {
            /* 跳过没有填写的花色牌，这张可能是其它花色的，不在这里 */
            continue;
        }
        if (PrePoint == CARD_POINTT_Unknow)
        {
            PrePoint =  FlushCards[index].Point;
            continue;
        }

        if (FlushCards[index].Point == PrePoint + 1)
        {
            MaxIndex = index;
            StriaghtNum ++;
        }
        else
        {
            StriaghtNum = 1;
        }
        PrePoint = FlushCards[index].Point;
    }

    //printf("result %d %d;\r\n", StriaghtNum, MaxIndex);

    if ((StriaghtNum >= 5) && (FlushCards[MaxIndex].Point == CARD_POINTT_A))
    {
        *pMaxPoint = FlushCards[MaxIndex].Point;
        return CARD_TYPES_Royal_Flush;
    }
    if (StriaghtNum >= 5)
    {
        *pMaxPoint = FlushCards[MaxIndex].Point;
        return CARD_TYPES_Straight_Flush;
    }
    return CARD_TYPES_None;
}

/* 根据不同的牌数，计算牌的类型 */
CARD_TYPES STG_GetCardTypes(CARD *pCards, int CardNum, CARD_POINT MaxPoints[CARD_TYPES_Butt])
{
    int AllPoints[CARD_POINTT_BUTT];// = {0};
    int AllColors[CARD_COLOR_BUTT];// = {0};
    CARD FlushCards[CARD_COLOR_BUTT][8];    /* 不超过7张牌 */
    int index = 0;
    int Pairs = 0;
    int Three = 0;
    int Four = 0;
    int Straight = 0;
    CARD_TYPES CardType = CARD_TYPES_High;
    CARD_TYPES ColorType = CARD_TYPES_None;
    CARD_COLOR FlushColor = CARD_COLOR_Unknow;

    memset(&AllPoints, 0, sizeof(AllPoints));
    memset(&AllColors, 0, sizeof(AllColors));
    memset(FlushCards, 0, sizeof(FlushCards));

    STG_SortCardByPoint(pCards, CardNum);

    /* 单牌最大的就是最后一张 */
    MaxPoints[CARD_TYPES_High] = pCards[CardNum - 1].Point;

    //TRACE("STG_GetCardTypes:%d; \r\n", CardNum);

    for (index = 0; index < CardNum; index ++)
    {
        AllColors[pCards[index].Color] ++;
        AllPoints[pCards[index].Point] ++;

        /* 将牌放到同一花色数组中 */
        FlushCards[pCards[index].Color][index] = pCards[index];
        //
        if (AllColors[pCards[index].Color] >= 5)
        {
            /* 记录同花，同花顺，皇家同花顺的最大点数 */
            MaxPoints[CARD_TYPES_Flush] = pCards[index].Point;
            FlushColor = pCards[index].Color;
            ColorType = CARD_TYPES_Flush;
        }
    }

    //TRACE("STG_GetCardTypes:\r\n");
    for (index = CARD_POINTT_2; index < CARD_POINTT_A + 1; index ++)
    {
        if (AllPoints[index] == 0)
        {
            /* 顺子只要中间有一个断开，就不是了，但如果已经大于5了，就不变 */
            Straight = Straight >=5 ? Straight : 0;
        }
        if (AllPoints[index] >= 1)
        {
            //TRACE("[%d]%d ", index, AllPoints[index]);
            Straight ++;
            if (Straight >= 5)
            {
                MaxPoints[CARD_TYPES_Straight] = (CARD_POINT)index;
            }
            if ((index == CARD_POINTT_5) && (Straight == 4) && (AllPoints[CARD_POINTT_A] > 0))
            {
                /* A,2,3,4,5的顺子 */
                static int spical_straight = 0;
                Straight ++;
                MaxPoints[CARD_TYPES_Straight] = (CARD_POINT)index;
                //printf("spical_straight:%d\r\n", spical_straight++);
            }
        }
        if (AllPoints[index] == 2)
        {
            Pairs ++;
            MaxPoints[CARD_TYPES_OnePair] = (CARD_POINT)index;
            if (Pairs >= 2)
            {
                MaxPoints[CARD_TYPES_TwoPair] = (CARD_POINT)index;
            }
            if (Three >= 1)
            {
                MaxPoints[CARD_TYPES_Full_House] = (CARD_POINT)index;
            }
        }
        if (AllPoints[index] == 3)
        {
            Three ++;
            MaxPoints[CARD_TYPES_Three_Of_A_Kind] = (CARD_POINT)index;
            if ((Pairs >= 1) || (Three >= 2))
            {
                MaxPoints[CARD_TYPES_Full_House] = (CARD_POINT)index;
            }
        }
        if (AllPoints[index] == 4)
        {
            Four ++;
            MaxPoints[CARD_TYPES_Four_Of_A_Kind] = (CARD_POINT)index;
        }
    }

    /* 赢牌类型从大到小判断 */

    /* 同花顺和皇家同花顺，暂时不判断，概率为0 */
    if ((Straight >= 5) && (ColorType == CARD_TYPES_Flush))
    {
        CARD_POINT SpicalMaxPoint;
        CARD_TYPES SpicalType = SET_GetStraightType(FlushCards[FlushColor],
                                                    7,
                                                    &SpicalMaxPoint);
        if (SpicalType != CARD_TYPES_None)
        {
            MaxPoints[SpicalType] = SpicalMaxPoint;
            return SpicalType;
        }
    }

//    TRACE("\r\nSTG_GetCardTypes_end. four:%d;three:%d;pair:%d;straight:%d;color:%d;\r\n",
//           Four, Three, Pairs, Straight, (int)ColorType);

    if (Four > 0)
    {
        return CARD_TYPES_Four_Of_A_Kind;
    }

    if (((Three > 0) && (Pairs > 0)) || (Three >= 2))
    {
        return CARD_TYPES_Full_House;
    }

    if (ColorType == CARD_TYPES_Flush)
    {
        return CARD_TYPES_Flush;
    }

    if (Straight >= 5)
    {
        return CARD_TYPES_Straight;
    }

    if (Three > 0)
    {
        return CARD_TYPES_Three_Of_A_Kind;
    }

    if (Pairs >= 2)
    {
        return CARD_TYPES_TwoPair;
    }

    if (Pairs >= 1)
    {
        return CARD_TYPES_OnePair;
    }

    return CARD_TYPES_High;
}

void STG_AnalyseWinCard_AllCards(CARD AllCards[7], int WinIndex, int PlayerNum)
{
    CARD_POINT MaxPoints[CARD_TYPES_Butt] = {(CARD_POINT)0};
    CARD_TYPES CardType = CARD_TYPES_None;
    CardType = STG_GetCardTypes(AllCards, 7, MaxPoints);
    //
    g_stg.AllWinCards[PlayerNum - 1][CardType][MaxPoints[CardType]].ShowTimes ++;
    if (WinIndex == 1)
    {
        g_stg.AllWinCards[PlayerNum - 1][CardType][MaxPoints[CardType]].WinTimes ++;
    }
//    printf("Save analyse data:[%d]Type %s, max %d, index %d;\r\n",
//           PlayerNum,
//           Msg_GetCardTypeName(CardType),
//          (int)MaxPoints[CardType], WinIndex);
    return;
}

/* 分析各选手的牌与公牌的组合，然后记录赢牌和出现牌的次数 */
void STG_AnalyseWinCard(RoundInfo *pRound)
{
    CARD AllCards[7] = {(CARD_POINT)0};
    int index = 0;
    MSG_SHOWDWON_PLAYER_CARD * pPlayCard = NULL;

//    printf("pRound->ShowDown.PlayerNum =%d;\r\n",  pRound->ShowDown.PlayerNum );
    for (index = 0; index < pRound->ShowDown.PlayerNum - 1; index ++)
    {
        /* 牌型算法要对AllCards排序，需要每次都重新赋值 */
        memcpy(&AllCards, &pRound->ShowDown.PublicCards, sizeof(pRound->ShowDown.PublicCards));
        pPlayCard = &pRound->ShowDown.Players[index];
        AllCards[5] = pPlayCard->HoldCards[0];
        AllCards[6] = pPlayCard->HoldCards[1];
        //TRACE("Debug_PrintShowDown:%d player %d;\r\n", pRound->RoundIndex, index);
        //Debug_PrintShowDown(&pRound->ShowDown);
        //Debug_PrintChardInfo(AllCards, 7);
        //
        STG_AnalyseWinCard_AllCards(AllCards,
                                    pRound->ShowDown.Players[index].Index,
                                    pRound->ShowDown.PlayerNum);
    }
}

/* 动态分析对手行为 */
void STG_AnalysePlayer(RoundInfo *pRound)
{
    /*  */

    return;
}

/* 分析牌型和对手情况 */
void * STG_ProcessThread(void *pArgs)
{
    int ret = 0;
    RoundInfo *pRound = NULL;
    queue_entry * pQueueEntry = NULL;

    printf("STG_ProcessThread.\r\n");

    while (g_stg.RunningFlag == true)
    {
        /* 有新的round数据 */
        //pRound = STG_GetNextRound();
        pQueueEntry = QueueGet(&g_round_queue);
        if (pQueueEntry == NULL)
        {
            usleep(1000);
            continue;
        }

        pRound = (RoundInfo *)pQueueEntry->pObjData;

        //Debug_ShowRoundInfo(pRound);

        /* 分析赢牌数据 */
        STG_AnalyseWinCard(pRound);

        /* 分析对手行为 */
        STG_AnalysePlayer(pRound);

        /* 释放一局的内存 */
        free(pQueueEntry);
        pQueueEntry = NULL;
        pRound = NULL;

        /* 分析老的数据 */
    }
    return NULL;
}

void STG_Init(void)
{
    int ret;

    QueueInit(&g_round_queue);

    /* 加载学习数据 */
    STG_LoadStudyData();
    g_stg.RunningFlag = true;
    pthread_mutex_init(&g_stg.Lock, NULL);

    ret = pthread_create(&g_stg.subThread, NULL, STG_ProcessThread, NULL);
    if (ret == -1)
    {
        printf("STG_Init pthread_create error!%d \r\n", errno);
    }

    pthread_detach(g_stg.subThread);
    return;
}

void STG_Dispose(void)
{
    g_stg.RunningFlag = false;
    pthread_cancel(g_stg.subThread);
    STG_SaveStudyData();
    return;
}

