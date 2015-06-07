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

void Debug_PrintChardInfo(CARD * pCard, int CardNum);
void Msg_LinerReader_Seat(char Buffer[256], RoundInfo * pArg);
void Msg_LinerReader_Blind(char Buffer[256], RoundInfo * pArg);
void Msg_LinerReader_Hold(char Buffer[256], RoundInfo * pArg);
void Msg_LinerReader_Inquire(char Buffer[256], RoundInfo * pArg);
void Msg_LinerReader_Flop(char Buffer[256], RoundInfo * pArg);
void Msg_LinerReader_Turn(char Buffer[256], RoundInfo * pArg);
void Msg_LinerReader_River(char Buffer[256], RoundInfo * pArg);
void Msg_LinerReader_ShowDown(char Buffer[256], RoundInfo * pArg);
void Msg_LinerReader_Notify(char Buffer[256], RoundInfo * pArg);
void Msg_LinerReader_GameOver(char Buffer[256], RoundInfo * pArg);
void Msg_LinerReader_PotWin(char Buffer[256], RoundInfo * pArg);

/*
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
*/
MSG_NAME_TYPE_ENTRY AllMsgTypes[] =
{
    {"" , "", 0, SER_MSG_TYPE_none, NULL},

    /***************************************************************************
    seat/ eol
    button: pid jetton money eol
    small blind: pid jetton money eol
    (big blind: pid jetton money eol)0-1
    (pid jetton money eol)0-5
    /seat eol
    ****************************************************************************/
    {"seat/ \n" , "/seat \n", sizeof("seat/ \n") - 1, SER_MSG_TYPE_seat_info, Msg_LinerReader_Seat},

    /****************************************************************************
    blind/ eol
    (pid: bet eol)1-2
    /blind eol
    ****************************************************************************/
    {"blind/ \n" , "/blind \n", sizeof("blind/ \n") - 1, SER_MSG_TYPE_blind, Msg_LinerReader_Blind},

    /****************************************************************************
    hold/ eol
    color point eol
    color point eol
    /hold eol
    ****************************************************************************/
    {"hold/ \n" , "/hold \n", sizeof("hold/ \n") - 1, SER_MSG_TYPE_hold_cards, Msg_LinerReader_Hold},

    /****************************************************************************
    inquire/ eol
    (pid jetton money bet blind | check | call | raise | all_in | fold eol)1-8
    total pot: num eol
    /inquire eol
    ****************************************************************************/
    {"inquire/ \n" , "/inquire \n", sizeof("inquire/ \n") - 1, SER_MSG_TYPE_inquire, Msg_LinerReader_Inquire},

    /****************************************************************************
    flop/ eol
    color point eol
    color point eol
    color point eol
    /flop eol
    ****************************************************************************/
    {"flop/ \n" , "/flop \n", sizeof("flop/ \n") - 1, SER_MSG_TYPE_flop, Msg_LinerReader_Flop},

    /****************************************************************************
    turn/ eol
    color point eol
    /turn eol
    ****************************************************************************/
    {"turn/ \n" , "/turn \n", sizeof("turn/ \n") - 1, SER_MSG_TYPE_turn, Msg_LinerReader_Turn},

    /****************************************************************************
    river/ eol
    color point eol
    /river eol
    ****************************************************************************/
    {"river/ \n" , "/river \n", sizeof("river/ \n") - 1, SER_MSG_TYPE_river, Msg_LinerReader_River},

    /*****************************************************************************
    showdown/ eol
    common/ eol
    (color point eol)5
    /common eol
    (rank: pid color point color point nut_hand eol)2-8
    /showdown eol
    *****************************************************************************/
    {"showdown/ \n" , "/showdown \n", sizeof("showdown/ \n") - 1, SER_MSG_TYPE_showdown, Msg_LinerReader_ShowDown},

    /****************************************************************************
    pot-win/ eol
    (pid: num eol)0-8
    /pot-win eol
    *****************************************************************************/
    {"pot-win/ \n" , "/pot-win \n", sizeof("seat/ \n") - 1, SER_MSG_TYPE_pot_win, Msg_LinerReader_PotWin},

    /*****************************************************************************
    notify/ eol
    (pid jetton money bet blind | check | call | raise | all_in | fold eol)1-8
    total pot: num eol
    /notify eol
    *****************************************************************************/
    {"notify/ \n" , "/notify \n", sizeof("notify/ \n") - 1, SER_MSG_TYPE_notify, Msg_LinerReader_Notify},

    /*****************************************************************************
    game-over eol
    *****************************************************************************/
    {"game-over \n" , NULL, 0, SER_MSG_TYPE_game_over, Msg_LinerReader_GameOver},
};

SER_MSG_TYPES Msg_GetMsgTypeByMsgName(const char * pMsgName)
{
    int Count = sizeof(AllMsgTypes)/sizeof(MSG_NAME_TYPE_ENTRY);
    MSG_NAME_TYPE_ENTRY * pEntry = NULL;
    int index = 0;
    for (index = 1; index < Count; index ++)
    {
        pEntry = &AllMsgTypes[index];
        if (memcmp((void *)pMsgName, (void *)pEntry->pStartName, pEntry->NameLen) == 0)
        {
            //TRACE("[%s %s]\r\n", pMsgName, pEntry->pStartName);
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

    memset(OutLine, 0, 256);

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

/* button: 1002 2000 8000 \nsmall blind: 1003 2000 8000 \nbig blind: 1001 2000 8000 \n1004 2000 8000\n */
void Msg_ReadSeatInfo_PlayerSeat(void * pObj, char LineBuffer[256])
{
    #define BUTTON      "button: "
    #define BUTTON_FMT  "button: %s %d %d"
    #define SMALL       "small blind: "
    #define SMALL_FMT   "small blind: %s %d %d"
    #define BIG         "big blind: "
    #define BIG_FMT     "big blind: %s %d %d"
    #define PLAYER_FMT  "%s %d %d"
    MSG_SEAT_INFO * pSetInfo = (MSG_SEAT_INFO *)pObj;
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
    if (readnum != 3)
    {
        TRACE("sscanf error [%s];\r\n", LineBuffer);
        return ;
    }
    memcpy(pPlayerSeat->PlayerID, PlayerSeat.PlayerID, sizeof(PlayerSeat.PlayerID));
    pPlayerSeat->Jetton = PlayerSeat.Jetton;
    pPlayerSeat->Money = PlayerSeat.Money;
    pSetInfo->PlayerNum ++;
    return;
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
         default:return "err";
    }
}

bool Msg_ReadCardsInfo_OneCard(const char LineBuffer[256], CARD * pCard)
{
    #define SPADES      "SPADES "
    #define HEARTS      "HEARTS "
    #define CLUBS       "CLUBS "
    #define DIAMONDS    "DIAMONDS "
    char Point[4] = {0};
    int scan_num = 0;
    CARD_COLOR Color = CARD_COLOR_Unknow;

    //TRACE("%s",LineBuffer);

    if (memcmp((void *)LineBuffer, (void *)SPADES, sizeof(SPADES) - 1) == 0)
    {
        Color = CARD_COLOR_SPADES;
        scan_num = sscanf(LineBuffer + sizeof(SPADES) - 1, "%s", Point);
        goto OUT;
    }
    if (memcmp((void *)LineBuffer, (void *)HEARTS, sizeof(HEARTS) - 1) == 0)
    {
        Color = CARD_COLOR_HEARTS;
        scan_num = sscanf(LineBuffer + sizeof(HEARTS) - 1, "%s", Point);
        goto OUT;
    }
    if (memcmp((void *)LineBuffer, (void *)CLUBS, sizeof(CLUBS) - 1) == 0)
    {
        Color = CARD_COLOR_CLUBS;
        scan_num = sscanf(LineBuffer + sizeof(CLUBS) - 1, "%s", Point);
        goto OUT;
    }
    if (memcmp((void *)LineBuffer, (void *)DIAMONDS, sizeof(DIAMONDS) - 1) == 0)
    {
        Color = CARD_COLOR_DIAMONDS;
        scan_num = sscanf(LineBuffer + sizeof(DIAMONDS) - 1, "%s", Point);
    }

OUT:
    if (scan_num != 1)
    {
        TRACE("sscanf error [%s];\r\n", LineBuffer);
        return false;
    }
    pCard->Color = Color;
    pCard->Point = GetCardPoint(Point);
    return true;
}

void Msg_ReadCardsInfo_Card(void * pObj, char LineBuffer[256])
{
    MSG_CARD_INFO * pCards = (MSG_CARD_INFO * )pObj;

    if(Msg_ReadCardsInfo_OneCard(LineBuffer, &pCards->Cards[pCards->CardNum]))
    {
        pCards->CardNum ++;
    }
    return;
}

void Msg_ReadPlayJettonInfo(MSG_POT_WIN_INFO * pPotWinInfo, char LineBuffer[256])
{
    int ReadNum = 0;
    int scan_num = 0;
    char PID[32] = {0};
    int Jetton = 0;

    /* "pot-win/ \n1001: 250 \n/pot-win \n", */
    scan_num = sscanf(LineBuffer, "%s %d", PID, &Jetton);
    if (scan_num != 2)
    {
        TRACE("sscanf error [%s];\r\n", LineBuffer);
        return;
    }

    memcpy(pPotWinInfo->PlayerID, PID, 32);
    pPotWinInfo->Jetton = Jetton;

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

void Msg_ReadBlindInfo_Ex(char LineBuffer[256], MSG_BLIND_INFO * pBlindInfo)
{
    Msg_ReadPlayJettonInfo(&pBlindInfo->BlindPlayers[pBlindInfo->BlindNum], LineBuffer);
    pBlindInfo->BlindNum ++;
    return;
}

const char * GetCardColorName(CARD * pCard)
{
    switch (pCard->Color)
    {
        default: return "err";
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

bool Msg_ReadPlayerCardInfo(MSG_SHOWDWON_PLAYER_CARD * pPlayerCard, char LineBuffer[256])
{
    char CardPoint1[4] = {0};
    char CardColor1[32] = {0};
    char CardPoint2[4] = {0};
    char CardColor2[32] = {0};
    char CardType[32] = {0};
    int index = 0;

    int scan_num = 0;

    //printf("%s", LineBuffer);
    /* "1: 1001 SPADES 10 DIAMONDS 10 FULL_HOUSE", */
    scan_num = sscanf(LineBuffer, "%d: %s %s %s %s %s %s",
               &index,
               pPlayerCard->PlayerID,
               CardColor1,CardPoint1,
               CardColor2,CardPoint2,
               CardType);
    if (scan_num != 7)
    {
        TRACE("sscanf error [%s];\r\n", LineBuffer);
        return false;
    }
    pPlayerCard->Index = index ;
    memcpy(pPlayerCard->CardType, CardType, 32);
    pPlayerCard->HoldCards[0].Color = GetCardColor(CardColor1);
    pPlayerCard->HoldCards[0].Point = GetCardPoint(CardPoint1);
    pPlayerCard->HoldCards[1].Color = GetCardColor(CardColor2);
    pPlayerCard->HoldCards[1].Point = GetCardPoint(CardPoint2);
    return true;
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

void Msg_ReadInquireInfoEx(const char LineBuffer[256], MSG_INQUIRE_INFO * pInquire)
{
    char Actions[32] = {0};
    char PID[32] = {0};
    int Jetton = 0;
    int Money = 0;
    int Bet = 0;
    int scan_num = 0;
    MSG_INQUIRE_PLAYER_ACTION * pPlayerAction = NULL;

    pPlayerAction = &pInquire->PlayerActions[pInquire->PlayerNum];
    scan_num = sscanf(LineBuffer, "%s %d %d %d %s",
                      PID, &Jetton, &Money, &Bet, Actions);
    if (scan_num != 5)
    {
        TRACE("sscanf error [%s];\r\n", LineBuffer);
        return;
    }
    memcpy(pPlayerAction->PlayerID, PID, 32),
    pPlayerAction->Jetton = Jetton;
    pPlayerAction->Money = Money;
    pPlayerAction->Bet = Bet;
    pPlayerAction->Action = GetAction(Actions);
    pInquire->PlayerNum ++;
    return;
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

void Msg_LinerReader_Seat(char Buffer[256], RoundInfo * pRound)
{
    Msg_ReadSeatInfo_PlayerSeat(&pRound->SeatInfo, Buffer);
    return;
}

void Msg_LinerReader_Blind(char Buffer[256], RoundInfo * pRound)
{
    Msg_ReadBlindInfo_Ex(Buffer, &pRound->Blind);
    return;
}

void Msg_LinerReader_Hold(char Buffer[256], RoundInfo * pRound)
{
    Msg_ReadCardsInfo_Card(&pRound->HoldCards, Buffer);
    return;
}

void Msg_LinerReader_Inquire(char Buffer[256], RoundInfo * pRound)
{
    if (strstr(Buffer, "total pot") != NULL)
    {
        int Jetton = 0;
        sscanf(Buffer, "total pot:%d", &Jetton);
        pRound->Inquires[pRound->InquireCount].TotalPot = Jetton;
        pRound->InquireCount ++;
        {
            extern int m_socket_id;
            //TRACE("Response check.\r\n");
            const char* response = "check";
            send(m_socket_id, response, sizeof("check"), 0);
        }
        return;
    }
    Msg_ReadInquireInfoEx(Buffer, &pRound->Inquires[pRound->InquireCount]);
    return;
}

void Msg_LinerReader_Flop(char Buffer[256], RoundInfo * pRound)
{
    Msg_ReadCardsInfo_Card(&pRound->PublicCards, Buffer);
    return;
}

void Msg_LinerReader_Turn(char Buffer[256], RoundInfo * pRound)
{
    Msg_ReadCardsInfo_Card(&pRound->PublicCards, Buffer);
    return;
}

const char *Msg_GetCardTypeName(CARD_TYPES Type)
{
    const char * TypeNames[] =
    {
        "CARD_TYPES_None",
        "CARD_TYPES_High",
        "CARD_TYPES_OnePair",
        "CARD_TYPES_TwoPair",
        "CARD_TYPES_Three_Of_A_Kind",
        "CARD_TYPES_Straight",
        "CARD_TYPES_Flush",
        "CARD_TYPES_Full_House",
        "CARD_TYPES_Four_Of_A_Kind",
        "CARD_TYPES_Straight_Flush",
        "CARD_TYPES_Royal_Flush",
    };
    return TypeNames[Type];
}

void Msg_LinerReader_River(char Buffer[256], RoundInfo * pRound)
{
    CARD AllCards[7];
    CARD_POINT MaxPoint[CARD_TYPES_Butt];
    CARD_TYPES Type = CARD_TYPES_None;
    Msg_ReadCardsInfo_Card(&pRound->PublicCards, Buffer);

    AllCards[0] = pRound->PublicCards.Cards[0];
    AllCards[1] = pRound->PublicCards.Cards[1];
    AllCards[2] = pRound->PublicCards.Cards[2];
    AllCards[3] = pRound->PublicCards.Cards[3];
    AllCards[4] = pRound->PublicCards.Cards[4];
    AllCards[5] = pRound->HoldCards.Cards[0];
    AllCards[6] = pRound->HoldCards.Cards[1];

    //Debug_PrintChardInfo(&pRound->PublicCards);
    //Debug_PrintChardInfo(&pRound->HoldCards);

    Type = STG_GetCardTypes(AllCards, 7, MaxPoint);
    //Debug_PrintChardInfo(AllCards, 7);

    //printf("Type %s, Max:%d\r\n", Msg_GetCardTypeName(Type), MaxPoint[Type]);

    return;
}

void Msg_LinerReader_ShowDown(char Buffer[256], RoundInfo * pRound)
{
    //TRACE("%s %d %d \r\n", Buffer, pRound->ShowDown.CardNum, pRound->ShowDown.PlayerNum);
    /* 先读取5张公牌 */
    if ((pRound->ShowDown.CardNum >= 1) && (pRound->ShowDown.CardNum <= 5))
    {
        CARD *pCard = NULL;
        pCard = &pRound->ShowDown.PublicCards[pRound->ShowDown.CardNum - 1];
        if (Msg_ReadCardsInfo_OneCard(Buffer, pCard))
        {
            pRound->ShowDown.CardNum ++;
        }
        return;
    }

    /* 读取每个人的手牌 */
    if ((pRound->ShowDown.PlayerNum >= 1) && (pRound->ShowDown.PlayerNum <= 8))
    {
        MSG_SHOWDWON_PLAYER_CARD *pPlayerCard = NULL;
        pPlayerCard = &pRound->ShowDown.Players[pRound->ShowDown.PlayerNum - 1];
        if (Msg_ReadPlayerCardInfo(pPlayerCard, Buffer))
        {
            pRound->ShowDown.PlayerNum ++;
        }
        return;
    }

    /* 开始读取公牌 */
    if (strstr(Buffer, "common/") != NULL)
    {
        pRound->ShowDown.CardNum = 1;
        return;
    }

    /* 结束读取公牌，开始读手牌 */
    if (strstr(Buffer, "/common ") != NULL)
    {
        pRound->ShowDown.PlayerNum = 1;
        return;
    }

    return;
}

void Msg_LinerReader_Notify(char Buffer[256], RoundInfo * pRound)
{
    return;
}

void Msg_LinerReader_GameOver(char Buffer[256], RoundInfo * pRound)
{
    /* 可以将数据保存到磁盘 */
    return;
}

void Msg_LinerReader_PotWin(char Buffer[256], RoundInfo * pRound)
{
    static int RoundIndex = 0;

    /* 读取彩池信息 */
    Msg_ReadPlayJettonInfo(&pRound->PotWin, Buffer);
    //
    TRACE("====save round %d =========\r\n", pRound->RoundIndex);
//  Debug_ShowRoundInfo(pRound);
    STG_SaveRoundData(pRound);

    RoundIndex ++;
    TRACE("=============Round %d =========\r\n", RoundIndex);
    memset(pRound, 0, sizeof(RoundInfo));
    pRound->RoundIndex = RoundIndex;
    //
    return;
}

void Msg_Read_Ex(const char * pMsg, int MaxLen, RoundInfo * pRound)
{
    int ReadNum;
    char Buffer[256] ;//= {0};
    MSG_READ_INFO MsgInfo = {0};
    SER_MSG_TYPES Type;//= Msg_GetMsgType(pMsg, MaxLen);
    MSG_NAME_TYPE_ENTRY * pMsgEntry;// = AllMsgTypes[Type];
    //
    MsgInfo.pMsg = pMsg;
    MsgInfo.MaxLen = MaxLen;

    while ((ReadNum = Msg_ReadLine(&MsgInfo, Buffer)) > 0)
    {
        Type = Msg_GetMsgTypeByMsgName(Buffer);
        pMsgEntry = &AllMsgTypes[Type];
        pRound->RoundStatus = Type;
        Msg_SetOffset(&MsgInfo, MsgInfo.Index + ReadNum);
//        TRACE("%s [%d]\r\n", Buffer, ReadNum);
        while ((ReadNum = Msg_ReadLine(&MsgInfo, Buffer)) > 0)
        {
            //TRACE("%s [%d]\r\n", Buffer, ReadNum);
            if (strcmp(Buffer, pMsgEntry->pEndName) == 0)
            {
                Msg_SetOffset(&MsgInfo, MsgInfo.Index + ReadNum);
                break;
            }
            if (pMsgEntry->LinerReader)
            {
                pMsgEntry->LinerReader(Buffer, pRound);
            }
            Msg_SetOffset(&MsgInfo, MsgInfo.Index + ReadNum);
        }
    }
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

void Debug_PrintSeatInfo(MSG_SEAT_INFO * pSeatInfo)
{
    int index = 0;
    PLAYER_SEAT_INFO * pPlayer = NULL;
    printf("Seat Info:\r\n");
    for (index = 0; index < pSeatInfo ->PlayerNum; index ++)
    {
        pPlayer = &pSeatInfo->Players[index];
        printf("%s ID:%s Jetton:%d Money:%d\n", Msg_GetPlayType(pPlayer), pPlayer->PlayerID, pPlayer->Jetton, pPlayer->Money);
    }
    return;
}

void Debug_PrintChardInfo(CARD * pCard, int CardNum)
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

void Debug_PrintBlindInfo(MSG_BLIND_INFO * pBlind)
{
    int index = 0;
    printf("Blind Info:\r\n");
    for (index = 0; index < pBlind->BlindNum; index ++)
    {
        printf("%s %d\r\n", pBlind->BlindPlayers[index].PlayerID,
               pBlind->BlindPlayers[index].Jetton);
    }
}

void Debug_PrintShowDown(MSG_SHOWDWON_INFO *pShowDown)
{
    int index = 0;
    for (index = 0; index < pShowDown->CardNum - 1; index ++)
    {
        CARD * pCard = NULL;
        pCard = &pShowDown->PublicCards[index];
        TRACE("%s %s\r\n",
               GetCardColorName(pCard),
               GetCardPointName(pCard->Point));
    }
    for (index = 0; index < pShowDown->PlayerNum - 1; index ++)
    {
        MSG_SHOWDWON_PLAYER_CARD * pPlayerCard = NULL;
        pPlayerCard = &pShowDown->Players[index];
        TRACE("%d %s %s %s %s %s\r\n",
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
    printf("===============round %d=================\r\n", pRound->RoundIndex);
    Debug_PrintShowDown(&pRound->ShowDown);
    printf("===============round %d=================\r\n", pRound->RoundIndex);
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
        //Msg_Read(TestMessages[index], strlen(TestMessages[index]) + 1, NULL, &roundInfo);
    }

    memset(&roundInfo, 0, sizeof(roundInfo));
    Msg_Read_Ex(TestMessages[10], strlen(TestMessages[10]), &roundInfo);
    Debug_PrintSeatInfo(&roundInfo.SeatInfo);
    Debug_PrintBlindInfo(&roundInfo.Blind);
    Debug_PrintChardInfo(roundInfo.HoldCards.Cards, 2);
    //
    printf("========show down============\r\n");
    Msg_Read_Ex(TestMessages[7], strlen(TestMessages[7]), &roundInfo);
    Debug_PrintShowDown(&roundInfo.ShowDown);
    return 0;
}


