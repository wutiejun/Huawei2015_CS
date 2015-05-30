#include <stdio.h>
#include <stdlib.h>  //
#include <string.h>  //strlen
#include <unistd.h>  //usleep/close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Player.h"

/* 负责消息读取与解析 */

RoundInfo roundInfo = {0};

MSG_NAME_TYPE_ENTRY AllMsgTypes[] =
{
    /***************************************************************************
    seat/ eol
    button: pid jetton money eol
    small blind: pid jetton money eol
    (big blind: pid jetton money eol)0-1
    (pid jetton money eol)0-5
    /seat eol
    ****************************************************************************/
    {"seat/ \n" , "/seat \n", SER_MSG_TYPE_seat_info},

    /****************************************************************************
    hold/ eol
    color point eol
    color point eol
    /hold eol
    ****************************************************************************/
    {"blind/ \n" , "/blind \n", SER_MSG_TYPE_blind},

    /****************************************************************************
    hold/ eol
    color point eol
    color point eol
    /hold eol
    ****************************************************************************/
    {"hold/ \n" , "/hold \n", SER_MSG_TYPE_hold_cards},

    /****************************************************************************
    inquire/ eol
    (pid jetton money bet blind | check | call | raise | all_in | fold eol)1-8
    total pot: num eol
    /inquire eol
    ****************************************************************************/
    {"inquire/ \n" , "/inquire \n", SER_MSG_TYPE_inquire},

    /****************************************************************************
    flop/ eol
    color point eol
    color point eol
    color point eol
    /flop eol
    ****************************************************************************/
    {"flop/ \n" , "/flop \n", SER_MSG_TYPE_flop},

    /****************************************************************************
    turn/ eol
    color point eol
    /turn eol
    ****************************************************************************/
    {"turn/ \n" , "/turn \n", SER_MSG_TYPE_turn},

    /****************************************************************************
    river/ eol
    color point eol
    /river eol
    ****************************************************************************/
    {"river/ \n" , "/river \n", SER_MSG_TYPE_river},

    /*****************************************************************************
    showdown/ eol
    common/ eol
    (color point eol)5
    /common eol
    (rank: pid color point color point nut_hand eol)2-8
    /showdown eol
    *****************************************************************************/
    {"showdown/ \n" , "/showdown \n", SER_MSG_TYPE_showdown},

    /****************************************************************************
    pot-win/ eol
    (pid: num eol)0-8
    /pot-win eol
    *****************************************************************************/
    {"pot-win/ \n" , "/pot-win \n", SER_MSG_TYPE_pot_win},

    /*****************************************************************************
    notify/ eol
    (pid jetton money bet blind | check | call | raise | all_in | fold eol)1-8
    total pot: num eol
    /notify eol
    *****************************************************************************/
    {"notify/ \n" , "/notify \n", SER_MSG_TYPE_notify},

    /*****************************************************************************
    game-over eol
    *****************************************************************************/
    {"game-over \n" , NULL, SER_MSG_TYPE_game_over},
};

SER_MSG_TYPES Msg_GetMsgTypeByMsgName(const char * pMsgName)
{
    int Count = sizeof(AllMsgTypes)/sizeof(MSG_NAME_TYPE_ENTRY);
    MSG_NAME_TYPE_ENTRY * pEntry = NULL;
    int index = 0;
    for (index = 0; index < Count; index ++)
    {
        pEntry = &AllMsgTypes[index];
        if (memcmp((void *)pMsgName, (void *)pEntry->pStartName, strlen(pEntry->pStartName)) == 0)
        {
            return pEntry->MsgType;
        }
    }
    return SER_MSG_TYPE_none;
}

const char * Msg_GetMsgNameByType(SER_MSG_TYPES Type)
{
    int Count = sizeof(AllMsgTypes)/sizeof(MSG_NAME_TYPE_ENTRY);
    MSG_NAME_TYPE_ENTRY * pEntry = NULL;
    int index = 0;
    for (index = 0; index < Count; index ++)
    {
        pEntry = &AllMsgTypes[index];
        if (Type == pEntry->MsgType)
        {
            return pEntry->pStartName;
        }
    }
    return "";
}

/*
    从消息中读取一行，如果没有读取到，就返回-1，否则返回这一行的起始位置。
*/
int Msg_ReadLine(MSG_READ_INFO * pInfo, char OutLine[256])
{
    int index = pInfo->Index;
    int ReadNum = 0;

    /* 找到一行的结束\n */
    while (index++ < pInfo->MaxLen)
    {
        if (pInfo->pMsg[index] == '\n')
        {
            /* 复制数据 */
            ReadNum = index - pInfo->Index + 1;
            memcpy((void *)OutLine, (void *)(pInfo->pMsg + pInfo->Index), ReadNum);
            break;
        }
    }
    return ReadNum;
}

/* 设置读取偏移 */
void Msg_SetOffset(MSG_READ_INFO * pInfo, int Index)
{
    Index = (Index < 0) ? 0 : Index;
    Index = (Index > pInfo->MaxLen) ? pInfo->MaxLen : Index;
    pInfo->Index = Index;
    return;
}

/* 读取消息第一行，取得消息类型 */
SER_MSG_TYPES Msg_GetMsgType(const char * pMsg, int MaxLen)
{
    char LineBuffer[256] = {0};
    int index = 0;
    MSG_READ_INFO MsgInfo = {0};

    MsgInfo.pMsg = pMsg;
    MsgInfo.MaxLen = MaxLen;
    index = Msg_ReadLine(&MsgInfo, LineBuffer);
    //TRACE("Msg_ReadLine:%d:[%s];\n", index, LineBuffer);
    if (index < 0)
    {
        return SER_MSG_TYPE_none;
    }
    return Msg_GetMsgTypeByMsgName(LineBuffer);
}

/* button: 1002 2000 8000 \nsmall blind: 1003 2000 8000 \nbig blind: 1001 2000 8000 \n1004 2000 8000\n */
void Msg_ReadSeatInfo_PlayerSeat(MSG_SEAT_INFO * pSetInfo, char LineBuffer[256])
{
    #define BUTTON      "button: "
    #define BUTTON_FMT  "button: %s %d %d"
    #define SMALL       "small blind: "
    #define SMALL_FMT   "small blind: %s %d %d"
    #define BIG         "big blind: "
    #define BIG_FMT     "big blind: %s %d %d"
    #define PLAYER_FMT  "%s %d %d"
    const char * pFmt = NULL;
    PLAYER_SEAT_INFO * pPlayerSeat = NULL;
    PLAYER_SEAT_INFO PlayerSeat = {0};
    int readnum = 0;

    if (memcmp((void *)LineBuffer, (void *)BUTTON, sizeof(BUTTON) - 1) == 0)
    {
        pFmt = BUTTON_FMT;
        pPlayerSeat = &pSetInfo->Players[0];
        pPlayerSeat->Type = PLAYER_SEAT_TYPES_button;
        goto SCAN_PLAYER;
    }

    if (memcmp((void *)LineBuffer, (void *)SMALL, sizeof(SMALL) - 1) == 0)
    {
        pFmt = SMALL_FMT;
        pPlayerSeat = &pSetInfo->Players[1];
        pPlayerSeat->Type = PLAYER_SEAT_TYPES_small_blind;
        goto SCAN_PLAYER;
    }

    if (memcmp((void *)LineBuffer, (void *)BIG, sizeof(BIG) - 1) == 0)
    {
        pFmt = BIG_FMT;
        pPlayerSeat = &pSetInfo->Players[2];
        pPlayerSeat->Type = PLAYER_SEAT_TYPES_big_blind;
        goto SCAN_PLAYER;
    }

    pFmt = PLAYER_FMT;
    pPlayerSeat = &pSetInfo->Players[pSetInfo->PlayerNum];
    pPlayerSeat->Type = PLAYER_SEAT_TYPES_none;

SCAN_PLAYER:
    readnum = sscanf(LineBuffer, pFmt, PlayerSeat.PlayerID, &PlayerSeat.Jetton, &PlayerSeat.Money);
    if (readnum == 3)
    {
        memcpy(pPlayerSeat->PlayerID, PlayerSeat.PlayerID, sizeof(PlayerSeat.PlayerID));
        pPlayerSeat->Jetton = PlayerSeat.Jetton;
        pPlayerSeat->Money = PlayerSeat.Money;
        pSetInfo->PlayerNum ++;
    }
    return;
}

/* 读取seat类型消息 */
int Msg_ReadSeatInfo(MSG_READ_INFO *pMsgInfo, MSG_SEAT_INFO * pSetInfo)
{
    char LineBuffer[256] = {0};
    int ReadNum = 0;
    int Line = 0;

    /* 先读取消息类型，第一行 */
    ReadNum = Msg_ReadLine(pMsgInfo, LineBuffer);
    Line ++;
    //printf("%s\r\n", LineBuffer);
    while (ReadNum > 0)
    {
        memset(LineBuffer, 0, sizeof(LineBuffer));
        Msg_SetOffset(pMsgInfo, pMsgInfo->Index + ReadNum);
        ReadNum = Msg_ReadLine(pMsgInfo, LineBuffer);
        //printf("%s\r\n", LineBuffer);
        Line ++;
        //
        Msg_ReadSeatInfo_PlayerSeat(pSetInfo, LineBuffer);
    }
    return 0;
}

 CARD_POINT GetCardPoint(char CardPoint[4])
 {
     switch (CardPoint[0])
     {
         case '2' : return CARD_POINTT_2;
         case '3' : return CARD_POINTT_3;
         case '4' : return CARD_POINTT_4;
         case '5' : return CARD_POINTT_5;
         case '6' : return CARD_POINTT_6;
         case '7' : return CARD_POINTT_7;
         case '8' : return CARD_POINTT_8;
         case '9' : return CARD_POINTT_9;
         case '1' : return CARD_POINTT_10;  /* 如果是1的话，必然是10 */
         case 'J' : return CARD_POINTT_J;
         case 'Q' : return CARD_POINTT_Q;
         case 'K' : return CARD_POINTT_K;
         case 'A' : return CARD_POINTT_A;
     }
 }

const char * GetCardPointName(CARD_POINT point)
{
    switch (point)
    {
         case CARD_POINTT_2: return "2" ;
         case CARD_POINTT_3: return "3" ;
         case CARD_POINTT_4: return "4" ;
         case CARD_POINTT_5: return "5" ;
         case CARD_POINTT_6: return "6" ;
         case CARD_POINTT_7: return "7" ;
         case CARD_POINTT_8: return "8" ;
         case CARD_POINTT_9: return "9" ;
         case CARD_POINTT_10:return "10";
         case CARD_POINTT_J: return "J" ;
         case CARD_POINTT_Q: return "Q" ;
         case CARD_POINTT_K: return "K" ;
         case CARD_POINTT_A: return "A" ;
    }
 }

void Msg_ReadCardsInfo_Card(MSG_CARD_INFO * pCards, char LineBuffer[256])
{
    #define SPADES      "SPADES "
    #define HEARTS      "HEARTS "
    #define CLUBS       "CLUBS "
    #define DIAMONDS    "DIAMONDS "
    CARD * pCard = NULL;
    char Point[4] = {0};

    if (memcmp((void *)LineBuffer, (void *)SPADES, sizeof(SPADES) - 1) == 0)
    {
        pCard = &pCards->Cards[pCards->CardNum];
        pCard->Color = CARD_COLOR_SPADES;
        sscanf(LineBuffer + sizeof(SPADES) - 1, "%s", Point);
        pCard->Point = GetCardPoint(Point);
        pCards->CardNum ++;
        return;
    }
    if (memcmp((void *)LineBuffer, (void *)HEARTS, sizeof(HEARTS) - 1) == 0)
    {
        pCard = &pCards->Cards[pCards->CardNum];
        pCard->Color = CARD_COLOR_HEARTS;
        sscanf(LineBuffer + sizeof(HEARTS) - 1, "%s", Point);
        pCard->Point = GetCardPoint(Point);
        pCards->CardNum ++;
        return;
    }
    if (memcmp((void *)LineBuffer, (void *)CLUBS, sizeof(CLUBS) - 1) == 0)
    {
        pCard = &pCards->Cards[pCards->CardNum];
        pCard->Color = CARD_COLOR_CLUBS;
        sscanf(LineBuffer + sizeof(CLUBS) - 1, "%s", Point);
        pCard->Point = GetCardPoint(Point);
        pCards->CardNum ++;
        return;
    }
    if (memcmp((void *)LineBuffer, (void *)DIAMONDS, sizeof(DIAMONDS) - 1) == 0)
    {
        pCard = &pCards->Cards[pCards->CardNum];
        pCard->Color = CARD_COLOR_DIAMONDS;
        sscanf(LineBuffer + sizeof(DIAMONDS) - 1, "%s", Point);
        pCard->Point = GetCardPoint(Point);
        pCards->CardNum ++;
        return;
    }
    return;
}

/* 读取牌消息，包括公牌，转牌，河牌才手牌 */
int Msg_ReadCardsInfo(MSG_READ_INFO *pMsgInfo, MSG_CARD_INFO * pCards)
{
    char LineBuffer[256] = {0};
    int ReadNum = 0;
    int Line = 0;

    /* 先读取消息类型，第一行 */
    ReadNum = Msg_ReadLine(pMsgInfo, LineBuffer);
    Line ++;
    //printf("%s\r\n", LineBuffer);
    while (ReadNum > 0)
    {
        memset(LineBuffer, 0, sizeof(LineBuffer));
        Msg_SetOffset(pMsgInfo, pMsgInfo->Index + ReadNum);
        ReadNum = Msg_ReadLine(pMsgInfo, LineBuffer);
        //printf("%s\r\n", LineBuffer);
        Line ++;
        //
        Msg_ReadCardsInfo_Card(pCards, LineBuffer);
    }
    return 0;
}

void Msg_ReadPlayJettonInfo(MSG_POT_WIN_INFO * pPotWinInfo, char LineBuffer[256])
{
    int ReadNum = 0;

    /* "pot-win/ \n1001: 250 \n/pot-win \n", */
    sscanf(LineBuffer, "%s %d", pPotWinInfo->PlayerID, &pPotWinInfo->Jetton);

    //printf("%s", LineBuffer);

    //TRACE("%d \r\n", roundInfo.ShowDown.PlayerNum);

    ReadNum = strlen(pPotWinInfo->PlayerID);
    while (ReadNum >= 0)
    {
        if (pPotWinInfo->PlayerID[ReadNum] == ':')
        {
            pPotWinInfo->PlayerID[ReadNum] = '\0';
            break;
        }
        ReadNum --;
    }
    //printf("%s[%d]\r\n", pPotWinInfo->PlayerID, pPotWinInfo->Jetton);
    return;
}

int Msg_ReadPotWinInfo(MSG_READ_INFO *pMsgInfo, MSG_POT_WIN_INFO * pPotWinInfo)
{
    char LineBuffer[256] = {0};
    int ReadNum = 0;

    /* 先读取消息类型，第一行 */
    ReadNum = Msg_ReadLine(pMsgInfo, LineBuffer);
    Msg_SetOffset(pMsgInfo, pMsgInfo->Index + ReadNum);
    memset(LineBuffer, 0, sizeof(LineBuffer));
    ReadNum = Msg_ReadLine(pMsgInfo, LineBuffer);

    Msg_ReadPlayJettonInfo(pPotWinInfo, LineBuffer);

    //printf("%s[%d]\r\n", LineBuffer, pPotWinInfo->Jetton);
    return 0;
}

int Msg_ReadBlindInfo(MSG_READ_INFO *pMsgInfo, MSG_BLIND_INFO * pBlindInfo)
{
    char LineBuffer[256] = {0};
    int ReadNum = 0;

    /* 先读取消息类型，第一行 */
    ReadNum = Msg_ReadLine(pMsgInfo, LineBuffer);
    Msg_SetOffset(pMsgInfo, pMsgInfo->Index + ReadNum);

    while (memcmp(LineBuffer, "/blind", sizeof("/blind") - 1) != 0)
    {
        memset(LineBuffer, 0, sizeof(LineBuffer));
        ReadNum = Msg_ReadLine(pMsgInfo, LineBuffer);
        if (memcmp(LineBuffer, "/blind", sizeof("/blind") - 1) == 0)
        {
            break;
        }
        Msg_SetOffset(pMsgInfo, pMsgInfo->Index + ReadNum);
        Msg_ReadPlayJettonInfo(&pBlindInfo->BlindPlayers[pBlindInfo->BlindNum], LineBuffer);
        pBlindInfo->BlindNum ++;
    }

    //printf("%s[%d]\r\n", LineBuffer, pPotWinInfo->Jetton);
    return 0;
}



const char * GetCardColorName(CARD * pCard)
{
    switch (pCard->Color)
    {
        default:
        case CARD_COLOR_CLUBS:      return "CLUBS";
        case CARD_COLOR_DIAMONDS:   return "DIAMONDS";
        case CARD_COLOR_HEARTS:     return "HEARTS";
        case CARD_COLOR_SPADES:     return "SPADES";
    }
}


CARD_COLOR GetCardColor(const char *pColor)
{
    if (strcmp("CLUBS", pColor) == 0) return CARD_COLOR_CLUBS;
    if (strcmp("DIAMONDS", pColor) == 0) return CARD_COLOR_DIAMONDS;
    if (strcmp("HEARTS", pColor) == 0) return CARD_COLOR_HEARTS;
    if (strcmp("SPADES", pColor) == 0) return CARD_COLOR_SPADES;
    return CARD_COLOR_Unknow;
}

void Msg_ReadPlayerCardInfo(MSG_SHOWDWON_PLAYER_CARD * pPlayerCard, char LineBuffer[256])
{
    char CardPoint1[4] = {0};
    char CardColor1[32] = {0};
    char CardPoint2[4] = {0};
    char CardColor2[32] = {0};

    //printf("%s", LineBuffer);
    /* "1: 1001 SPADES 10 DIAMONDS 10 FULL_HOUSE", */
    sscanf(LineBuffer, "%d: %s %s %s %s %s %s",
           &pPlayerCard->Index,
           pPlayerCard->PlayerID,
           CardColor1,CardPoint1,
           CardColor2,CardPoint2,
           pPlayerCard->CardType);
    pPlayerCard->HoldCards[0].Color = GetCardColor(CardColor1);
    pPlayerCard->HoldCards[0].Point = GetCardPoint(CardPoint1);
    pPlayerCard->HoldCards[1].Color = GetCardColor(CardColor2);
    pPlayerCard->HoldCards[1].Point = GetCardPoint(CardPoint2);
    return;
}

int Msg_ReadShowDownInfo(MSG_READ_INFO *pMsgInfo, MSG_SHOWDWON_INFO * pShowDown)
{
    char LineBuffer[256] = {0};
    int ReadNum = 0;
    const char * pStart = NULL;

    pStart = strstr(pMsgInfo->pMsg, "/common \n");
    if (pStart == NULL)
    {
        return 0;
    }

    pStart += sizeof("/common \n") - 1;
    ReadNum = pStart - pMsgInfo->pMsg;
    Msg_SetOffset(pMsgInfo, pMsgInfo->Index + ReadNum);

    //printf("%d\r\n", pShowDown->PlayerNum);

    while (memcmp(LineBuffer, "/showdown", sizeof("/showdown") - 1) != 0)
    {
        memset(LineBuffer, 0, sizeof(LineBuffer));
        ReadNum = Msg_ReadLine(pMsgInfo, LineBuffer);
        if (memcmp(LineBuffer, "/showdown", sizeof("/showdown") - 1) == 0)
        {
            break;
        }
        //printf("%s", LineBuffer);
        Msg_SetOffset(pMsgInfo, pMsgInfo->Index + ReadNum);
        Msg_ReadPlayerCardInfo(&pShowDown->Players[pShowDown->PlayerNum], LineBuffer);
        pShowDown->PlayerNum ++;
        //printf("%d\r\n", pShowDown->PlayerNum);
    }
    //pShowDown->PlayerNum --;
    return 0;
}

PLAYER_Action GetAction(const char * ActionNAme)
{
    if (strcmp(ActionNAme, "call") == 0) return ACTION_call;
    if (strcmp(ActionNAme, "check") == 0) return ACTION_check;
    if (strcmp(ActionNAme, "raise") == 0) return ACTION_raise;
    if (strcmp(ActionNAme, "fold") == 0) return ACTION_fold;
    if (strcmp(ActionNAme, "allin") == 0) return ACTION_allin;
    return ACTION_fold;
}

const char * GetActionName(PLAYER_Action act)
{
    const char * Actions[] = {"check", "all", "allin", "rais", "fold"};
    return Actions[act];
}

/*
inquire/ eol
(pid jetton money bet blind | check | call | raise | all_in | fold eol)1-8
total pot: num eol
/inquire eol
1003 1900 8000 100 blind
*/
int Msg_ReadInquireInfo(MSG_READ_INFO *pMsgInfo, MSG_INQUIRE_INFO * pInquire)
{
    char LineBuffer[256] = {0};
    char Actions[32] = {0};
    int ReadNum = 0;
    MSG_INQUIRE_PLAYER_ACTION * pPlayerAction = NULL;

    /* 先读取消息类型，第一行 inquire */
    ReadNum = Msg_ReadLine(pMsgInfo, LineBuffer);
    Msg_SetOffset(pMsgInfo, pMsgInfo->Index + ReadNum);

    while (memcmp(LineBuffer, "total pot", sizeof("total pot") - 1) != 0)
    {
        //printf("Msg_ReadBlindInfo:%s", LineBuffer);
        memset(LineBuffer, 0, sizeof(LineBuffer));
        ReadNum = Msg_ReadLine(pMsgInfo, LineBuffer);
        if (memcmp(LineBuffer, "total pot", sizeof("total pot") - 1) == 0)
        {
            break;
        }
        Msg_SetOffset(pMsgInfo, pMsgInfo->Index + ReadNum);
        pPlayerAction = &pInquire->PlayerActions[pInquire->PlayerNum];
        sscanf(LineBuffer, "%s %d %d %d %s",
               pPlayerAction->PlayerID,
               &pPlayerAction->Jetton,
               &pPlayerAction->Money,
               &pPlayerAction->Bet,
               Actions);
        pPlayerAction->Action = GetAction(Actions);
        pInquire->PlayerNum ++;

        #if 0
        printf("%s %d %d %d %s",
                pPlayerAction->PlayerID,
               pPlayerAction->Jetton,
               pPlayerAction->Money,
               pPlayerAction->Bet,
               GetActionName(pPlayerAction->Action));
        #endif
    }
    //pInquire->PlayerNum --;

    /* 读取total pot */
    sscanf(LineBuffer, "total pot:%d", &pInquire->TotalPot);
    //printf("totol pot [%d]\r\n", pInquire->TotalPot);
    return 0;
}

/* 根据坐位类型返回坐位信息 */
const char * Msg_GetPlayType(PLAYER_SEAT_INFO *pPlayerInfo)
{
    switch (pPlayerInfo->Type)
    {
        default:
        case PLAYER_SEAT_TYPES_none:        return "common";
        case PLAYER_SEAT_TYPES_small_blind: return "small blind";
        case PLAYER_SEAT_TYPES_big_blind:   return "big blind";
        case PLAYER_SEAT_TYPES_button:      return "Button";
    }
}

SER_MSG_TYPES Msg_Read(const char * pMsg, int MaxLen, void * pData, RoundInfo * pRound)
{
    int ret = 0;
    SER_MSG_TYPES Type = Msg_GetMsgType(pMsg, MaxLen);
    MSG_READ_INFO MsgInfo = {0};

    MsgInfo.pMsg = pMsg;
    MsgInfo.MaxLen = MaxLen;

    //printf("get msg:%d %s\r\n", Type, Msg_GetMsgNameByType(Type));
    switch (Type)
    {
        case SER_MSG_TYPE_seat_info:
            ret = Msg_ReadSeatInfo(&MsgInfo, &pRound->SeatInfo);
            break;
        case SER_MSG_TYPE_blind:
            ret = Msg_ReadBlindInfo(&MsgInfo, &pRound->Blind);
            break;
        case SER_MSG_TYPE_flop:
            ret = Msg_ReadCardsInfo(&MsgInfo, &pRound->PublicCards);
            break;
        case SER_MSG_TYPE_turn:
            ret = Msg_ReadCardsInfo(&MsgInfo, &pRound->PublicCards);
            break;
        case SER_MSG_TYPE_river:
            ret = Msg_ReadCardsInfo(&MsgInfo, &pRound->PublicCards);
            break;
        case SER_MSG_TYPE_hold_cards:
            ret = Msg_ReadCardsInfo(&MsgInfo, &pRound->HoldCards);
            break;
        case SER_MSG_TYPE_inquire:
            ret = Msg_ReadInquireInfo(&MsgInfo, &pRound->Inquires[pRound->InquireCount]);
            pRound->InquireCount = (pRound->InquireCount + 1) % MAX_INQUIRE_COUNT;
            break;
        case SER_MSG_TYPE_showdown:
            ret = Msg_ReadShowDownInfo(&MsgInfo, &pRound->ShowDown);
            break;
        case SER_MSG_TYPE_pot_win:
            Msg_ReadPotWinInfo(&MsgInfo, &pRound->PotWin);
            break;
        default:
            break;
    }
    //TRACE("%d \r\n", pRound->ShowDown.PlayerNum);
    return Type;
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
    /* 7 */ "showdown/ \ncommon/ SPADES 2 \nSPADES 3 \nDIAMONDS 4 \nHEARTS 10 \nSPADES 4 \n/common \n1: 1001 SPADES 10 DIAMONDS 10 FULL_HOUSE \n2: 1002 SPADES 7 DIAMONDS Q ONE_PAIR \n/showdown \n",
    /* 8 */ "pot-win/ \n1001: 250 \n/pot-win \n",
    /* 9 */ "game-over \n"
};

void Debug_PrintSeatInfo(MSG_SEAT_INFO * pSeatInfo)
{
    int index = 0;
    PLAYER_SEAT_INFO * pPlayer = NULL;
    for (index = 0; index < pSeatInfo ->PlayerNum; index ++)
    {
        pPlayer = &pSeatInfo->Players[index];
        printf("%s ID:%s Jetton:%d Money:%d\n", Msg_GetPlayType(pPlayer), pPlayer->PlayerID, pPlayer->Jetton, pPlayer->Money);
    }
    return;
}

void Debug_PrintPublicChardInfo(MSG_CARD_INFO * pPublicCard)
{
    int index = 0;
    CARD * pCard = NULL;
    for (index = 0; index < pPublicCard->CardNum; index ++)
    {
        pCard = &pPublicCard->Cards[index];
        printf("card_%d %s %s\n", index, GetCardColorName(pCard),
               GetCardPointName(pCard->Point));
    }
    return;
}
#ifdef DEBUG
int main(int argc, char * argv[])
#else
int main_ex(int argc, char * argv[])
#endif
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
        Msg_Read(TestMessages[index], strlen(TestMessages[index]) + 1, NULL, &roundInfo);

    }

    /* 测试seat消息解析 */
    printf("test seat info:\r\n");
    MsgInfo.pMsg = TestMessages[0];
    MsgInfo.MaxLen = strlen(TestMessages[0]);
    if(Msg_ReadSeatInfo(&MsgInfo, &SeatInfo) == 0)
    {
        Debug_PrintSeatInfo(&SeatInfo);
    }

    /* 测试flop，turn，以及rirver牌的消息 */
    printf("test public card info:\r\n");
    memset(&MsgInfo, 0, sizeof(MsgInfo));
    MsgInfo.pMsg = TestMessages[4];
    MsgInfo.MaxLen = strlen(TestMessages[4]);
    Msg_ReadCardsInfo(&MsgInfo, &PublicCards);

    memset(&MsgInfo, 0, sizeof(MsgInfo));
    MsgInfo.pMsg = TestMessages[5];
    MsgInfo.MaxLen = strlen(TestMessages[5]);
    Msg_ReadCardsInfo(&MsgInfo, &PublicCards);

    memset(&MsgInfo, 0, sizeof(MsgInfo));
    MsgInfo.pMsg = TestMessages[6];
    MsgInfo.MaxLen = strlen(TestMessages[6]);
    Msg_ReadCardsInfo(&MsgInfo, &PublicCards);

    Debug_PrintPublicChardInfo(&PublicCards);

    /* 手牌解析 */
    {
        MSG_CARD_INFO HoldCards = {0};
        printf("test hold card info:\r\n");
        memset(&MsgInfo, 0, sizeof(MsgInfo));
        MsgInfo.pMsg = TestMessages[2];
        MsgInfo.MaxLen = strlen(TestMessages[2]);
        Msg_ReadCardsInfo(&MsgInfo, &HoldCards);
        Debug_PrintPublicChardInfo(&HoldCards);
    }

    {
        MSG_POT_WIN_INFO PotWin = {0};
        printf("test pot win info:\r\n");
        memset(&MsgInfo, 0, sizeof(MsgInfo));
        MsgInfo.pMsg = TestMessages[8];
        MsgInfo.MaxLen = strlen(TestMessages[8]);
        Msg_ReadPotWinInfo(&MsgInfo, &PotWin);
        printf("%s %d\r\n", PotWin.PlayerID, PotWin.Jetton);
    }

    {
        MSG_BLIND_INFO BlindInfo = {0};
        printf("test blind info:\r\n");
        memset(&MsgInfo, 0, sizeof(MsgInfo));
        MsgInfo.pMsg = TestMessages[1];
        MsgInfo.MaxLen = strlen(TestMessages[1]);
        Msg_ReadBlindInfo(&MsgInfo, &BlindInfo);
        printf("blind num %d\r\n", BlindInfo.BlindNum);
        for (index = 0; index < BlindInfo.BlindNum; index ++)
        {
            printf("%s %d\r\n", BlindInfo.BlindPlayers[index].PlayerID,
                   BlindInfo.BlindPlayers[index].Jetton);
        }
    }

    {
        /* show down msg */
        MSG_SHOWDWON_INFO showDown = {0};
        MSG_SHOWDWON_PLAYER_CARD * pPlayerCard = NULL;
        printf("test show down info:\r\n");
        memset(&MsgInfo, 0, sizeof(MsgInfo));
        MsgInfo.pMsg = TestMessages[7];
        MsgInfo.MaxLen = strlen(TestMessages[7]);
        Msg_ReadShowDownInfo(&MsgInfo, &showDown);
        for (index = 0; index < showDown.PlayerNum; index ++)
        {
            pPlayerCard = &showDown.Players[index];
            printf("%d %s %s %s %s %s\r\n",
                   pPlayerCard->Index,
                   GetCardColorName(&pPlayerCard->HoldCards[0]),
                   GetCardPointName(pPlayerCard->HoldCards[0].Point),
                   GetCardColorName(&pPlayerCard->HoldCards[1]),
                   GetCardPointName(pPlayerCard->HoldCards[1].Point),
                   pPlayerCard->CardType);
        }
    }

    {
        /* for inquire msg */
        MSG_INQUIRE_INFO Inquire = {0};
        printf("test inquire info:\r\n");
        memset(&MsgInfo, 0, sizeof(MsgInfo));
        MsgInfo.pMsg = TestMessages[3];
        MsgInfo.MaxLen = strlen(TestMessages[3]);
        Msg_ReadInquireInfo(&MsgInfo, &Inquire);
        for (index = 0; index < Inquire.PlayerNum; index ++)
        {
            printf("%s %d %d %d %s\r\n",
                   Inquire.PlayerActions[index].PlayerID,
                   Inquire.PlayerActions[index].Jetton,
                   Inquire.PlayerActions[index].Money,
                   Inquire.PlayerActions[index].Bet,
                   GetActionName(Inquire.PlayerActions[index].Action));
        }
        printf("Total bet:%d\r\n", Inquire.TotalPot);
    }

    return 0;
}


