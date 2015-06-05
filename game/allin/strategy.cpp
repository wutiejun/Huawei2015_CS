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

#define MAX_ROUND_COUNT 2000

/* 负责处理策略 */

/* 赢牌类型 */
typedef struct STG_WIN_CARDS_
{
    int ShowTimes;
    int WinTimes;
    CARD_TYPES WinType;     /* 赢牌的类型 */
    CARD_POINT MaxPoint;    /* 赢牌的最大点数 */
    CARD_POINT SedPoint;    /* 赢牌的次大点数 */
} STG_WIN_CARDS;

typedef struct STG_DATA_
{
    int RunningFlag;
    int StgWriteIndex;
    int StgReadIndex;

    pthread_mutex_t Lock;
    RoundInfo Rounds[MAX_ROUND_COUNT]; /* 20M */

    /* 赢牌的数据 */
    STG_WIN_CARDS AllWinCards[CARD_TYPES_Butt][CARD_POINTT_BUTT];
} STG_DATA;

STG_DATA g_stg = {0};

static void STG_Lock(void)
{
    pthread_mutex_lock(&g_stg.Lock);
}

static void STG_Unlock(void)
{
    pthread_mutex_lock(&g_stg.Lock);
}

void STG_LoadStudyData(void)
{
    int fid = -1;
    int read_size = 0;

    memset(&g_stg.AllWinCards, 0, sizeof(g_stg.AllWinCards));

    fid = open("game_win_data.bin", O_WRONLY);
    if (fid == -1)
    {
        return;
    }

    read_size = read(fid, &g_stg.AllWinCards, sizeof(g_stg.AllWinCards));
    if (read_size != sizeof(g_stg.AllWinCards))
    {
        memset(&g_stg.AllWinCards, 0, sizeof(g_stg.AllWinCards));
    }
    close(fid);
    return;
}

void STG_SaveStudyData(void)
{
    int fid = -1;
    fid = open("game_win_data.bin", O_CREAT|O_WRONLY|O_TRUNC|O_SYNC);
    if (fid == -1)
    {
        return;
    }
    write(fid, &g_stg.AllWinCards, sizeof(g_stg.AllWinCards));
    close(fid);
    return;
}

void * STG_ProcessThread(void *pArgs)
{
    int ret = 0;
    while (g_stg.RunningFlag)
    {
        STG_Lock();

        STG_Unlock();
    }
    return NULL;
}

void STG_Init(void)
{
    int ret;
    pthread_t subThread;

    /* 加载学习数据 */
    STG_LoadStudyData();
    g_stg.RunningFlag = true;
    pthread_mutex_init(&g_stg.Lock, NULL);

    ret = pthread_create(&subThread, NULL, STG_ProcessThread, NULL);
    if (ret == -1)
    {
        printf("STG_Init pthread_create error!%d \r\n", errno);
    }

    pthread_detach(subThread);
    return;
}

void STG_Dispose(void)
{
    g_stg.RunningFlag = false;
    STG_Lock();
    return;
}

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

void STG_SaveRoundData(RoundInfo * pRoundInfo)
{
    STG_Lock();
    memcpy(&g_stg.Rounds[g_stg.StgWriteIndex], pRoundInfo, sizeof(RoundInfo));
    g_stg.StgWriteIndex = (g_stg.StgWriteIndex + 1) % MAX_ROUND_COUNT;
    STG_Unlock();
    return;
}

/* 根据不同的牌数，计算牌的类型 */
CARD_TYPES STG_GetCardTypes(CARD *pCards, int CardNum, CARD_POINT MaxPoints[CARD_TYPES_Butt])
{
    int AllPoints[CARD_POINTT_A + 1] = {0};
    int AllColors[CARD_COLOR_SPADES + 1] = {0};
    int index = 0;
    int Pairs = 0;
    int Three = 0;
    int Four = 0;
    int Straight = 0;
    CARD_TYPES CardType = CARD_TYPES_High;
    CARD_TYPES ColorType = CARD_TYPES_None;

    STG_SortCardByPoint(pCards, CardNum);

    /* 单牌最大的就是最后一张 */
    MaxPoints[CARD_TYPES_High] = pCards[CardNum - 1].Point;

    for (index = 0; index < CardNum; index ++)
    {
        AllColors[pCards[index].Color] ++;
        AllPoints[pCards[index].Point] ++;
        if (AllColors[pCards[index].Color] >= 5)
        {
            /* 记录同花，同花顺，皇家同花顺的最大点数 */
            MaxPoints[CARD_TYPES_Flush] = pCards[index].Point;
            MaxPoints[CARD_TYPES_Straight_Flush] = pCards[index].Point;
            MaxPoints[CARD_TYPES_Royal_Flush] = pCards[index].Point;
            ColorType = CARD_TYPES_Flush;
        }
    }

    for (index = CARD_POINTT_2; index < CARD_POINTT_A + 1; index ++)
    {
        switch (AllPoints[index])
        {
        case 1:
            Straight ++;
            if (Straight >= 5)
            {
                goto Straight;
            }
            break;
        case 2:
            Straight ++;
            if (Straight >= 5)
            {
                goto Straight;
            }
            Pairs ++;
            MaxPoints[CARD_TYPES_OnePair] = (CARD_POINT)index;
            CardType = CARD_TYPES_OnePair;
            if (Pairs >= 2)
            {
                MaxPoints[CARD_TYPES_TwoPair] = (CARD_POINT)index;
                CardType = CARD_TYPES_TwoPair;
            }
            break;
        case 3:
            Straight ++;
            if (Straight >= 5)
            {
                goto Straight;
            }
            Three ++;
            MaxPoints[CARD_TYPES_Three_Of_A_Kind] = (CARD_POINT)index;
            CardType = CARD_TYPES_Three_Of_A_Kind;
            if (Pairs >= 1)
            {
                MaxPoints[CARD_TYPES_Full_House] = (CARD_POINT)index;
                CardType = CARD_TYPES_Full_House;
            }
            break;
        case 4:
            Four ++;
            MaxPoints[CARD_TYPES_Four_Of_A_Kind] = (CARD_POINT)index;
            CardType = CARD_TYPES_Four_Of_A_Kind;
            break;
        default:
            Straight = 0;
            break;
        }
    }

    /* 特殊的顺子，A,2,3,4,5 */
    #if 0
    if ()
    {
        CardType = CARD_TYPES_Straight;
    }
    #endif
    return CardType;

Straight:
    MaxPoints[CARD_TYPES_Straight] = (CARD_POINT)index;
    CardType = CARD_TYPES_Straight;
    if (ColorType == CARD_TYPES_Flush)
    {
        CardType = CARD_TYPES_Straight_Flush;
        if (MaxPoints[CARD_TYPES_Royal_Flush] == CARD_POINTT_A)
        {
            CardType = CARD_TYPES_Royal_Flush;
        }
    }

    return CardType;

}



