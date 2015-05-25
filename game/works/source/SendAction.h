#pragma once
//#include <winsock2.h>
#include <sys/socket.h>
#include "ChooseStrategy.h"

extern int my_sock_client;

void SendAction(const StrategyInof &info);