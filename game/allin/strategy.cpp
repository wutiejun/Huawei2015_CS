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

/* 各类牌出现的机率 */
double g_CardTypeRate_1[CARD_TYPES_Royal_Flush + 1] =
{
    0, 1/1.002, 1/1.25, 1/20, 1/46, 1/254, 1/508, 1/693, 1/4164, 1/64973, 1/649739
};

double g_CardTypeRate_2[CARD_TYPES_Royal_Flush + 1] =
{
    0, 1/1.002, 1/1.25, 1/20, 1/46, 1/254, 1/508, 1/693, 1/4164, 1/64973, 1/649739
};

typedef struct STG_DATA_
{
    int StgIndex;
    pthread_mutex_t Lock;
    RoundInfo Rounds[1024]; /* 13M */
} STG_DATA;

STG_DATA g_stg;

static void STG_Lock(void)
{
    pthread_mutex_lock(&g_stg.Lock);
}

static void STG_Unlock(void)
{
    pthread_mutex_lock(&g_stg.Lock);
}

void STG_Init(void)
{

    return;
}

void STG_SaveRoundData(RoundInfo * pRoundInfo)
{
    STG_Lock();
    memcpy(&g_stg.Rounds[g_stg.StgIndex], pRoundInfo, sizeof(RoundInfo));
    g_stg.StgIndex ++;
    STG_Unlock();
    return;
}



