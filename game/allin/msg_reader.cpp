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
void Msg_ReadSeatInfo_EndAction(RoundInfo *pRound);
void Msg_Inquire_Action(RoundInfo * pRound);
void STG_Inquire_Action(RoundInfo * pRound);
void STG_SaveRoundData(RoundInfo * pRound);
void ExitGame(RoundInfo * pRound);

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
    {"seat/ \n" , "/seat \n", sizeof("seat/ \n") - 1,
    SER_MSG_TYPE_seat_info, Msg_LinerReader_Seat, Msg_ReadSeatInfo_EndAction},

    /****************************************************************************
    blind/ eol
    (pid: bet eol)1-2
    /blind eol
    ****************************************************************************/
    {"blind/ \n" , "/blind \n", sizeof("blind/ \n") - 1,
    SER_MSG_TYPE_blind, Msg_LinerReader_Blind, NULL},

    /****************************************************************************
    hold/ eol
    color point eol
    color point eol
    /hold eol
    ****************************************************************************/
    {"hold/ \n" , "/hold \n", sizeof("hold/ \n") - 1,
    SER_MSG_TYPE_hold_cards, Msg_LinerReader_Hold, NULL},

    /****************************************************************************
    inquire/ eol
    (pid jetton money bet blind | check | call | raise | all_in | fold eol)1-8
    total pot: num eol
    /inquire eol
    ****************************************************************************/
    {"inquire/ \n" , "/inquire \n", sizeof("inquire/ \n") - 1,
    SER_MSG_TYPE_inquire, Msg_LinerReader_Inquire, Msg_Inquire_Action},

    /****************************************************************************
    flop/ eol
    color point eol
    color point eol
    color point eol
    /flop eol
    ****************************************************************************/
    {"flop/ \n" , "/flop \n", sizeof("flop/ \n") - 1,
    SER_MSG_TYPE_flop, Msg_LinerReader_Flop, NULL},

    /****************************************************************************
    turn/ eol
    color point eol
    /turn eol
    ****************************************************************************/
    {"turn/ \n" , "/turn \n", sizeof("turn/ \n") - 1,
    SER_MSG_TYPE_turn, Msg_LinerReader_Turn, NULL},

    /****************************************************************************
    river/ eol
    color point eol
    /river eol
    ****************************************************************************/
    {"river/ \n" , "/river \n", sizeof("river/ \n") - 1,
    SER_MSG_TYPE_river, Msg_LinerReader_River, NULL},

    /*****************************************************************************
    showdown/ eol
    common/ eol
    (color point eol)5
    /common eol
    (rank: pid color point color point nut_hand eol)2-8
    /showdown eol
    *****************************************************************************/
    {"showdown/ \n" , "/showdown \n", sizeof("showdown/ \n") - 1,
    SER_MSG_TYPE_showdown, Msg_LinerReader_ShowDown, NULL},

    /****************************************************************************
    pot-win/ eol
    (pid: num eol)0-8
    /pot-win eol
    *****************************************************************************/
    {"pot-win/ \n" , "/pot-win \n", sizeof("seat/ \n") - 1,
    SER_MSG_TYPE_pot_win, Msg_LinerReader_PotWin, STG_SaveRoundData},

    /*****************************************************************************
    notify/ eol
    (pid jetton money bet blind | check | call | raise | all_in | fold eol)1-8
    total pot: num eol
    /notify eol
    *****************************************************************************/
    {"notify/ \n" , "/notify \n", sizeof("notify/ \n") - 1,
    SER_MSG_TYPE_notify, Msg_LinerReader_Notify, NULL},

    /*****************************************************************************
    game-over eol
    *****************************************************************************/
    {"game-over \n" , NULL, 0, SER_MSG_TYPE_game_over,
    Msg_LinerReader_GameOver, ExitGame},
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
    #define BUTTON_FMT  "button: %d %d %d"
    #define SMALL       "small blind: "
    #define SMALL_FMT   "small blind: %d %d %d"
    #define BIG         "big blind: "
    #define BIG_FMT     "big blind: %d %d %d"
    #define PLAYER_FMT  "%d %d %d"
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
    readnum = sscanf(LineBuffer, pFmt, &PlayerSeat.PlayerID, &PlayerSeat.Jetton, &PlayerSeat.Money);
    if (readnum != 3)
    {
        TRACE("sscanf error [%s];\r\n", LineBuffer);
        return ;
    }
    //memcpy(pPlayerSeat->PlayerID, PlayerSeat.PlayerID, sizeof(PlayerSeat.PlayerID));
    //pPlayerSeat->Jetton = PlayerSeat.Jetton;
    //pPlayerSeat->Money = PlayerSeat.Money;
    *pPlayerSeat = PlayerSeat;
    pSetInfo->PlayerNum ++;
    return;
}

void Msg_ReadSeatInfo_EndAction(RoundInfo *pRound)
{
    //pRound->SeatInfo.PlayerNum --;
    //printf("Read seat num:%d\r\n", pRound->SeatInfo.PlayerNum);
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

/* 读取Pot win或者blind消息，两个消息结构一样，可以用一个函数读取 */
/* "pot-win/ \n1001: 250 \n/pot-win \n", */
void Msg_ReadPlayJettonInfo(PLAYER_JETTION_INFO * pPlayerJetton, char LineBuffer[256])
{
    int ReadNum = 0;
    int scan_num = 0;
    //char PID[32] = {0};
    int PID =0;
    int Jetton = 0;

    scan_num = sscanf(LineBuffer, "%d: %d", &PID, &Jetton);
    if (scan_num != 2)
    {
        TRACE("sscanf error [%s];\r\n", LineBuffer);
        return;
    }
    //memcpy(pPotWinInfo->PlayerID, PID, 32);
    pPlayerJetton->PlayerID = PID;
    pPlayerJetton->Jetton = Jetton;
    //printf("Player %d %d.\r\n", PID, Jetton);
    return;
}

void Msg_ReadBlindInfo_Ex(char LineBuffer[256], MSG_BLIND_INFO * pBlindInfo)
{
    Msg_ReadPlayJettonInfo(&pBlindInfo->BlindPlayers[pBlindInfo->BlindNum], LineBuffer);
    pBlindInfo->BlindNum = (pBlindInfo->BlindNum + 1) % 2;
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
    scan_num = sscanf(LineBuffer, "%d: %d %s %s %s %s %s",
               &index,
               &pPlayerCard->PlayerID,
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

PLAYER_Action_EN GetAction(const char * ActionNAme)
{
    if (strcmp(ActionNAme, "call") == 0) return ACTION_call;
    if (strcmp(ActionNAme, "check") == 0) return ACTION_check;
    if (strcmp(ActionNAme, "raise") == 0) return ACTION_raise;
    if (strcmp(ActionNAme, "fold") == 0) return ACTION_fold;
    if (strcmp(ActionNAme, "allin") == 0) return ACTION_allin;
    return ACTION_fold;
}

const char * GetActionName(PLAYER_Action_EN act)
{
    const char * Actions[ACTION_BUTTON] = {"fold", "check", "call", "raise", "all_in", "fold"};
    return Actions[act];
}

/* 一次inquire msg中最多8个玩家的信息 */
void Msg_ReadInquireInfoEx(const char LineBuffer[256], MSG_INQUIRE_INFO * pInquire)
{
    char Actions[32] = {0};
    unsigned int PID = 0;
    int Jetton = 0;
    int Money = 0;
    int Bet = 0;
    int scan_num = 0;
    MSG_INQUIRE_PLAYER_ACTION * pPlayerAction = NULL;

    DOT(pInquire->PlayerNum);    

    printf("Msg_ReadInquireInfoEx:%s;\r\n", LineBuffer);
    
    pPlayerAction = &pInquire->PlayerActions[pInquire->PlayerNum];
    scan_num = sscanf(LineBuffer, "%d %d %d %d %s",
                      &PID, &Jetton, &Money, &Bet, Actions);
    if (scan_num != 5)
    {
        TRACE("sscanf error [%s];\r\n", LineBuffer);
        return;
    }

    //memcpy(pPlayerAction->PlayerID, PID, 32),
    pPlayerAction->PlayerID = PID;
    pPlayerAction->Jetton = Jetton;
    pPlayerAction->Money = Money;
    pPlayerAction->Bet = Bet;
    pPlayerAction->Action = GetAction(Actions);

    /* 最多只有8个，如果少于8个，则可能有同一ID的查询在里面，后面再分析处理 */
    pInquire->PlayerNum = (pInquire->PlayerNum + 1) % 8;
    DOT(pInquire->PlayerNum);
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
    pRound->RoundStatus = SER_MSG_TYPE_seat_info;
    return;
}

void Msg_LinerReader_Blind(char Buffer[256], RoundInfo * pRound)
{
    Msg_ReadBlindInfo_Ex(Buffer, &pRound->Blind);
    pRound->RoundStatus = SER_MSG_TYPE_blind;
    return;
}

void Msg_LinerReader_Hold(char Buffer[256], RoundInfo * pRound)
{
    Msg_ReadCardsInfo_Card(&pRound->HoldCards, Buffer);
    pRound->RoundStatus = SER_MSG_TYPE_hold_cards;
//    Debug_PrintChardInfo(__FILE__, __LINE__, pRound->HoldCards.Cards, pRound->HoldCards.CardNum);
    return;
}

MSG_INQUIRE_INFO * Msg_GetCurrentInquireInfo(RoundInfo * pRound)
{
    DOT(pRound->PublicCards.CardNum);
    switch (pRound->PublicCards.CardNum)
    {
        default:
        case 0:
            return &pRound->Inquires[0];
        case 3:
            return &pRound->Inquires[1];
        case 4:
            return &pRound->Inquires[2];
        case 5:
            return &pRound->Inquires[3];
    }
}

void Msg_LinerReader_Inquire(char Buffer[256], RoundInfo * pRound)
{
    MSG_INQUIRE_INFO * pInquireInfo = NULL;

    pInquireInfo = Msg_GetCurrentInquireInfo(pRound);
    if (strstr(Buffer, "total pot") != NULL)
    {
        int Jetton = 0;
        sscanf(Buffer, "total pot:%d", &Jetton);
        pInquireInfo->TotalPot = Jetton;
        return;
    }
    Msg_ReadInquireInfoEx(Buffer, pInquireInfo);
    //Debug_PrintChardInfo(__FILE__, __LINE__, pRound->HoldCards.Cards, pRound->HoldCards.CardNum);
    return;
}

void Msg_Inquire_Action(RoundInfo * pRound)
{
    #if 0
    char ActionBufer[128] = "check";
    ResponseAction(ActionBufer, strlen(ActionBufer));
    return;
    #else // CHECK
    return STG_Inquire_Action(pRound);
    #endif
}

void Msg_LinerReader_Flop(char Buffer[256], RoundInfo * pRound)
{
    Msg_ReadCardsInfo_Card(&pRound->PublicCards, Buffer);
    pRound->RoundStatus = SER_MSG_TYPE_flop;
    //Debug_PrintChardInfo(__FILE__, __LINE__, pRound->HoldCards.Cards, pRound->HoldCards.CardNum);
    return;
}

void Msg_LinerReader_Turn(char Buffer[256], RoundInfo * pRound)
{
    Msg_ReadCardsInfo_Card(&pRound->PublicCards, Buffer);
    pRound->RoundStatus = SER_MSG_TYPE_turn;
    //Debug_PrintChardInfo(__FILE__, __LINE__, pRound->HoldCards.Cards, pRound->HoldCards.CardNum);
    return;
}

const char *Msg_GetCardTypeName(CARD_TYPES Type)
{
    const char * TypeNames[] =
    {
        /* CT for CardType */
        "CT_None",
        "CT_HIGH_CARD",
        "CT_ONE_PAIR",
        "CT_TWO_PAIR",
        "CT_THREE_OF_A_KIND",
        "CT_STRAIGHT",
        "CT_FLUSH",
        "CT_FULL_HOUSE",
        "CT_FOUR_OF_A_KIND",
        "CT_STRAIGHT_FLUSH",
        "CT_ROYAL_FLUSH",
    };
    return TypeNames[Type];
}

void Msg_LinerReader_River(char Buffer[256], RoundInfo * pRound)
{
    Msg_ReadCardsInfo_Card(&pRound->PublicCards, Buffer);
    pRound->RoundStatus = SER_MSG_TYPE_river;
    
    //Debug_PrintChardInfo(__FILE__, __LINE__, pRound->HoldCards.Cards, pRound->HoldCards.CardNum);

    return;
}

void Msg_LinerReader_ShowDown(char Buffer[256], RoundInfo * pRound)
{
    pRound->RoundStatus = SER_MSG_TYPE_showdown;

    //printf("[%s:%d]%s;\r\n", __FILE__, __LINE__, Buffer);

#if 0
    /* 先读取5张公牌 */
    if ((pRound->ShowDown.CardNum >= 1) && (pRound->ShowDown.CardNum < 5))
    {
        CARD *pCard = NULL;
        pCard = &pRound->ShowDown.PublicCards[pRound->ShowDown.CardNum - 1];
        if (Msg_ReadCardsInfo_OneCard(Buffer, pCard))
        {
            pRound->ShowDown.CardNum ++;
        }
        return;
    }
#endif

    /* 读取每个人的手牌，个数不会超过seat时的个数 */
    if (   (pRound->ShowDown.PlayerNum >= 1))
    //    && (pRound->ShowDown.PlayerNum < pRound->SeatInfo.PlayerNum))
    {
        MSG_SHOWDWON_PLAYER_CARD *pPlayerCard = NULL;
        pPlayerCard = &pRound->ShowDown.Players[pRound->ShowDown.PlayerNum - 1];
        if (Msg_ReadPlayerCardInfo(pPlayerCard, Buffer))
        {
            pRound->ShowDown.PlayerNum ++;
        }
        Debug_PrintShowDown(&pRound->ShowDown);
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

/* 如果两个人的牌是一样的，pot win有两条或者多条数据，因此不能在解析一条数据以后就保存round数据 */
void Msg_LinerReader_PotWin(char Buffer[256], RoundInfo * pRound)
{
    static int RoundIndex = 0;
    /* 读取彩池信息 */
    Msg_ReadPlayJettonInfo(&pRound->PotWin.PotWin[pRound->PotWin.PotWinCount], Buffer);
//    printf("Player %d win %d;\r\n",
//           pRound->PotWin.PotWin[pRound->PotWin.PotWinCount].PlayerID,
//           pRound->PotWin.PotWin[pRound->PotWin.PotWinCount].Jetton);
    pRound->PotWin.PotWinCount = (pRound->PotWin.PotWinCount + 1) % 2;
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
        pRound->CurrentMsgType = Type;

        //DOT(pRound->CurrentMsgType);

        pMsgEntry = &AllMsgTypes[Type];
        
        Msg_SetOffset(&MsgInfo, MsgInfo.Index + ReadNum);
        //printf("[%d][%s]\r\n", ReadNum, Buffer);
        if (pRound->CurrentMsgType == SER_MSG_TYPE_game_over)
        {
            pMsgEntry->Action(pRound);
            break;
        }
        
        while ((ReadNum = Msg_ReadLine(&MsgInfo, Buffer)) > 0)
        {
            /* 读取消息结束，如果需要处理，就调用action处理 */
            if (strcmp(Buffer, pMsgEntry->pEndName) == 0)
            {
                Msg_SetOffset(&MsgInfo, MsgInfo.Index + ReadNum);
                if (pMsgEntry->Action)
                {
                    pMsgEntry->Action(pRound);
                }
                break;
            }

            /* 继续读取处理消息 */
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

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


