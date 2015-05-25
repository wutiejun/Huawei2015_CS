#include <stdio.h>
#include <stdlib.h>  //
#include <string.h>  //strlen
#include <unistd.h>  //usleep/close
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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



