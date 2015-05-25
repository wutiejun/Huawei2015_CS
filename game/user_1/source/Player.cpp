#include <time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <assert.h>
#include <list>
#include <map>
#include <string>
#include "Player.h"
#include "MsgProc.h"
#include "Global.h"

typedef int (*fun_ptr)(std::string msg);
 
const unsigned long MAX_LONG_MESSAGE_LEN = 1024;

static int my_player_id;
int my_sock_client;

std::map<std::string,fun_ptr> msgProcFunMap;

void initMsgProcFun()
{
    msgProcFunMap.insert(std::make_pair(SEAT_MSG,ProcSeatInfoMsg));
    msgProcFunMap.insert(std::make_pair(BLIND_MSG,ProcBlindMsg));
    msgProcFunMap.insert(std::make_pair(HOLD_MSG,ProcHoldCardMsg));
    msgProcFunMap.insert(std::make_pair(INQUIRE_MSG,ProcInquireMsg));
    msgProcFunMap.insert(std::make_pair(FLOP_MSG,ProcFolpMsg));
    msgProcFunMap.insert(std::make_pair(TURN_MSG,ProcTurnMsg));
    msgProcFunMap.insert(std::make_pair(RIVER_MSG,ProcRiverMsg));
    msgProcFunMap.insert(std::make_pair(SHOWDOWN_MSG,ProcShowdownMsg));
    msgProcFunMap.insert(std::make_pair(POT_WIN_MSG,ProcPotWinMsg));
}

int test_main( int argc, char *argv[] );

void GameStart(void)
{
    initMsgProcFun();

    InitGlobalData();

/*
    char *argv[] = {"xx", "tc","ac","6s","6h","6c"};//"3h","ah",

    test_main(6, argv);*/
}


void SetPlayerId(int id)
{
    my_player_id = id;
}

int GetPlayerId(void)
{
    return my_player_id;
}

int PlayerMsgProc(char* recv_buf)
{
    //将消息分类进行解析，
    std::string revMsg = recv_buf;
    //获取消息类型
    std::string::size_type begin = 0;
    std::string::size_type end = 0;

    if (revMsg.empty())
    {
        return 0;
    }

    //printf("\n revMsg: %s \n",revMsg.c_str());
    
    while (true)
    {
        std::string::size_type nameIndex = revMsg.find_first_of('/',begin);
        if (nameIndex == std::string::npos)//说明是GAME over消息
        {
            std::string leftMsg = revMsg.substr(begin);
            leftMsg = trim(leftMsg);

            if (leftMsg == GAME_OVER_MSG)
            {
                return GAME_OVER;
            }

            return 0;
        }
        
        std::string msgName = revMsg.substr(begin,nameIndex-begin);//需要去掉多余的空格

        
        msgName = trim(msgName);

        if (msgName.empty())
        {
            begin += 1;
            continue;
        }

        std::string msgEnd = "/"+ msgName;

        end = revMsg.find(msgEnd,begin);
        if (end  == std::string::npos)//说明解析出错
        {
            return 0;
        }

        begin = nameIndex - strlen(msgName.c_str());
        end = end + strlen(msgEnd.c_str());

        std::string oneMsg = revMsg.substr(begin,end-begin);
        begin = end;

        /*
        printf("%s \n",msgName.c_str());
        printf("\n oneMsg:%s,name:%s \n",oneMsg.c_str(),msgName.c_str());
        */
        if (msgProcFunMap.count(msgName) > 0)
        {
            msgProcFunMap[msgName](oneMsg.c_str());
        }             
    }

    return 0;
}

int RoundEntry(int& socket)
{
    my_sock_client = socket;

    char recv_buf[MAX_LONG_MESSAGE_LEN] = {'\0'};
    
    recv(socket,recv_buf,MAX_LONG_MESSAGE_LEN,0);

    return PlayerMsgProc(recv_buf);
}

int GameOver(void)
{
    return 0;
}