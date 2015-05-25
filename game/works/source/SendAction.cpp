#include "SendAction.h"
#include <stdio.h>
#include <string.h>

char* check_string = "check\n";
char* call_string = "call\n";
//char* raise_string = "raise";
char* all_string = "all in\n";
char* fold_string = "fold\n";

void sendActionCheck()
{
    send(my_sock_client, check_string,(int)strlen(check_string)+1, 0);
}

void sendActionCall()
{
    send(my_sock_client, call_string,(int)strlen(call_string)+1, 0);
}

void sendActionRaise(int num)
{
    char buf[200];

    sprintf(buf, "raise %d\n", num);
    send(my_sock_client, buf,(int)strlen(buf)+1, 0);
}

void sendActionAllin()
{
    send(my_sock_client, all_string,(int)strlen(all_string)+1, 0);
}

void sendActionFold()
{
    send(my_sock_client, fold_string,(int)strlen(fold_string)+1, 0);
}

void SendAction(const StrategyInof &info)
{
    if(info.action == ACTION_CHECK)
    {
        sendActionCheck();
    }
    else if (info.action == ACTION_CALL)
    {
        sendActionCall();
    }
    else if (info.action == ACTION_RAISE)
    {
        sendActionRaise(info.money);
    }
    else if (info.action == ACTION_ALL_IN)
    {
        sendActionAllin();
    }
    else if (info.action == ACTION_FOLD)
    {
        sendActionFold();
    }
    
}