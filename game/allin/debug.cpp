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
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>

#include <errno.h>
#include "Player.h"

pthread_mutex_t LogLock;

static void DebugWriteLog(const char * Msg, int size)
{
    static int log_file = -1;
    char LogFileName[64] = {0};
    if (log_file == -1)
    {
        sprintf(LogFileName, "./msg_log_%d.txt", (int)getpid());
        log_file = open(LogFileName, O_CREAT | O_WRONLY | O_TRUNC | O_SYNC);
    }
    //printf(Msg);
    write(log_file, Msg, size);
    syncfs(log_file);
    return;
}

void TRACE_Log(const char *file, int len, const char *fmt, ...)
{
    int n;
    int size = 1024;     /* Guess we need no more than 100 bytes. */
    char p[1024] = {0};
    struct timeval time;
    va_list ap;

    pthread_mutex_lock(&LogLock);
    
//    gettimeofday(&time, NULL);
//    n = snprintf(p, size, "[%d.%06d][%s:%d]",
//                 (int)time.tv_sec, (int)time.tv_usec, file, len);

    n = snprintf(p, size, "[%s:%d]", file, len);

    va_start(ap, fmt);
    n += vsnprintf(p + n, size, fmt, ap);
    va_end(ap);
    //printf("%s", p);
    DebugWriteLog(p, n);
    pthread_mutex_unlock(&LogLock);
    return;
}

const char  * TestMessages[] =
{
    /* 0 */ "seat/ \nbutton: 1002 2000 8000 \nsmall blind: 1003 2000 8000 \nbig blind: 1001 2000 8000\n1004 2000 8000 /seat \n",
    /* 1 */ "blind/ \n1003: 50 \n1001: 100 \n/blind \n",
    /* 2 */ "hold/ \nCLUBS J \nDIAMONDS 8 \n/hold \n",
    /* 3 */ "inquire/ \n1001 1900 8000 100 blind \n1003 1950 8000 50 blind \ntotal pot: 150 \n/inquire \n",
    /* 4 */ "flop/ \nSPADES A \nSPADES 3 \nDIAMONDS 4 \n/flop \n",
    /* 5 */ "turn/ \nHEARTS Q \n/turn \n",
    /* 6 */ "river/ \nSPADES K \n/river \n",
    /* 7 */ "showdown/ \ncommon/ \nSPADES 2 \nSPADES 3 \nDIAMONDS 4 \nHEARTS 10 \nSPADES 4 \n/common \n1: 1001 SPADES 10 DIAMONDS 10 FULL_HOUSE \n2: 1002 SPADES 7 DIAMONDS Q ONE_PAIR \n/showdown \n",
    /* 8 */ "pot-win/ \n1001: 250 \n/pot-win \n",
    /* 9 */ "game-over \n",
    /* 10 */ "seat/ \nbutton: 1002 2000 8000 \nsmall blind: 1003 2000 8000 \nbig blind: 1001 2000 8000\n1004 2000 8000 \n/seat \nblind/ \n1003: 50 \n1001: 100 \n/blind \nhold/ \nCLUBS J \nDIAMONDS 8 \n/hold \n",
};

void Debug_PrintInquireInfo(MSG_INQUIRE_INFO * pInquire)
{
    int index = 0;
    MSG_INQUIRE_PLAYER_ACTION * pPlayerAct = NULL;
    for (index = 0; index < pInquire->PlayerNum; index ++)
    {
        pPlayerAct = &pInquire->PlayerActions[index];
        printf("%d %d %d %d %s\r\n", 
               pPlayerAct->PlayerID, pPlayerAct->Jetton, pPlayerAct->Money, pPlayerAct->Bet, 
               GetActionName(pPlayerAct->Action));
    }
    printf("total pot: %d\r\n", pInquire->TotalPot);
}

void Debug_PrintSeatInfo(MSG_SEAT_INFO * pSeatInfo)
{
    int index = 0;
    PLAYER_SEAT_INFO * pPlayer = NULL;
    printf("Seat Info:\r\n");
    for (index = 0; index < pSeatInfo ->PlayerNum; index ++)
    {
        pPlayer = &pSeatInfo->Players[index];
        printf("%s ID:%d Jetton:%d Money:%d\n",
               Msg_GetPlayType(pPlayer),
               pPlayer->PlayerID,
               pPlayer->Jetton,
               pPlayer->Money);
    }
    return;
}

void Debug_PrintChardInfo(const char * pFile, int line, CARD * pCard, int CardNum)
{
    int index = 0;
    printf("[%s:%d]Card Info[%d]:\r\n", pFile, line, CardNum);
    for (index = 0; index < CardNum; index ++)
    {
        printf("card_%d %s %s\n", index, GetCardColorName(pCard),
               GetCardPointName(pCard->Point));
        pCard ++;
    }
    return;
}

void Debug_PrintBlindInfo(MSG_BLIND_INFO * pBlind)
{
    int index = 0;
    printf("Blind Info:\r\n");
    for (index = 0; index < pBlind->BlindNum; index ++)
    {
        printf("%d %d\r\n", pBlind->BlindPlayers[index].PlayerID,
               pBlind->BlindPlayers[index].Jetton);
    }
}

void Debug_PrintShowDown(MSG_SHOWDWON_INFO *pShowDown)
{
    int index = 0;
    printf("Debug_PrintShowDown[%d]:\r\n", pShowDown->PlayerNum);
    for (index = 0; index < pShowDown->CardNum - 1; index ++)
    {
        CARD * pCard = NULL;
        pCard = &pShowDown->PublicCards[index];
        printf("%s %s\r\n",
               GetCardColorName(pCard),
               GetCardPointName(pCard->Point));
    }
    for (index = 0; index < pShowDown->PlayerNum - 1; index ++)
    {
        MSG_SHOWDWON_PLAYER_CARD * pPlayerCard = NULL;
        pPlayerCard = &pShowDown->Players[index];
        printf("Player %d:%d %s %s %s %s %s\r\n",
               pPlayerCard->PlayerID,
               pPlayerCard->Index,
               GetCardColorName(&pPlayerCard->HoldCards[0]),
               GetCardPointName(pPlayerCard->HoldCards[0].Point),
               GetCardColorName(&pPlayerCard->HoldCards[1]),
               GetCardPointName(pPlayerCard->HoldCards[1].Point),
               pPlayerCard->CardType);
    }
    return;
}

void Debug_ShowRoundInfo(RoundInfo *pRound)
{
    //printf("===============round %d start=================\r\n", pRound->RoundIndex);
    TRACE("===============round %d start=================\r\n", pRound->RoundIndex);
    Debug_PrintShowDown(&pRound->ShowDown);
    TRACE("===============round %d end=================\r\n", pRound->RoundIndex);
    return;
}

#ifdef DEBUG


RoundInfo roundInfo = {0};

int ReadDebugMsgFromFile(const char * File, void *pBuffer, int Max)
{
    int Fid = -1;
    int RedSize = 0;
    Fid = open(File, O_RDONLY);
    if (Fid == -1)
    {
        printf("open error %d.\r\n", errno);
        return 0;
    }
    RedSize = read(Fid, pBuffer, Max);
    close(Fid);
    printf("read file %s size %d %s;\r\n", File, RedSize, strerror(errno));
    return RedSize;
}

void Test_001(void)
{
    int count = sizeof(TestMessages)/sizeof(const char  *);
    SER_MSG_TYPES type = SER_MSG_TYPE_none;
    int index = 0;
    MSG_READ_INFO MsgInfo = {0};
    MSG_SEAT_INFO SeatInfo = {0};
    MSG_CARD_INFO PublicCards = {0};

    printf("Round size is :%lu \r\n", sizeof(RoundInfo));

    for (index = 0; index < count; index++)
    {
        //type = Msg_GetMsgType(TestMessages[index], strlen(TestMessages[index]) + 1);
        //Msg_Read(TestMessages[index], strlen(TestMessages[index]) + 1, NULL, &roundInfo);
    }

    memset(&roundInfo, 0, sizeof(roundInfo));
    Msg_Read_Ex(TestMessages[10], strlen(TestMessages[10]), &roundInfo);
    Debug_PrintSeatInfo(&roundInfo.SeatInfo);
    Debug_PrintBlindInfo(&roundInfo.Blind);
    //Debug_PrintChardInfo(__FILE__, __LINE__, roundInfo.HoldCards.Cards, 2);
    //
    printf("========show down============\r\n");
    Msg_Read_Ex(TestMessages[7], strlen(TestMessages[7]), &roundInfo);
    Debug_PrintShowDown(&roundInfo.ShowDown);
    return ;
}

int m_socket_id=0;

void STG_AnalyseWinCard(RoundInfo *pRound);

int main(int argc, char * argv[])
{
    CARD AllCard[7];// = {0};
    char Buffer[4096] = {0};
    int readSize = 0;
    if (argc < 1)
    {
        printf("args error.\r\n");
        return 0;
    }
    readSize = ReadDebugMsgFromFile(argv[1], Buffer, 4096);
    if (readSize <= 0)
    {
        printf("ReadDebugMsgFromFile error.\r\n");
        return 0;
    }

    memset(AllCard, 0, sizeof(AllCard));

    Msg_Read_Ex(Buffer, readSize, &roundInfo);
    printf("%s\r\n", Buffer);

    Debug_PrintShowDown(&roundInfo.ShowDown);

    printf("\r\n");

    STG_AnalyseWinCard(&roundInfo);

    return 0;

}

#endif

