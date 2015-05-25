#include <stdio.h>
#include <stdlib.h>  //
#include <string.h>  //strlen
#include <unistd.h>  //usleep/close
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Player.h"

/* 负责牌型的处理和计算 */

/*****************************************************************************
 Function name: Card_Calculate
 Description  : 根据公牌和手牌来计算牌的结果
 Input        : CARD pPublicCards[3]    三张公牌(可能包括)
                CARD * pTurnCard        转牌
                CARD * pRiverCard       河牌
                CARD HandCards[2]       两张手牌
 Output       : None
 Return Value : 
    Successed : CARD_TYPES
    Failed    : 
*****************************************************************************/
CARD_TYPES Card_Calculate(CARD pPublicCards[3], CARD * pTurnCard, CARD * pRiverCard, CARD HandCards[2])
{
    CARD_TYPES CardType = CARD_TYPES_High;
    return CardType;
}

CARD_TYPES Card_CalculateEx(CARD * pPublicCards, int CardNum)
{
    CARD_TYPES CardType = CARD_TYPES_High;
    return CardType;
}


