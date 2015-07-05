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

#include <errno.h>
#include "Player.h"

int m_socket_id = -1;
int g_RunFlag = true;
RoundInfo LocalRoundInfo = {(SER_MSG_TYPES)0};

const char * g_Action = NULL;

bool server_msg_process(int size, const char* msg)
{
//    if (NULL != strstr(msg, "game-over"))
//    {
//        printf("My game over!\r\n");
//        return false;
//    }

    //SER_MSG_TYPES msg_type = Msg_Read(msg, size, NULL, &LocalRoundInfo);

    Msg_Read_Ex(msg, size, &LocalRoundInfo);

    //printf("Msg_Read:%d:%s\r\n", msg_type, Msg_GetMsgNameByType(msg_type));

    return true;
}


void ResponseAction(const char * pMsg, int size)
{
    int SendNum = 0;
    if (g_Action != NULL)
    {
        pMsg = g_Action;
        size = strlen(g_Action);
    }

    SendNum = send(m_socket_id, pMsg, size, 0);
    if (SendNum != size)
    {
        printf("ResponseAction error %d.", errno);
    }
    return;
}

#include <signal.h>
#include <execinfo.h>

 void OnSigSem(int signal)
 {
    int j, nptrs;
    #define SIZE 100
    void *buffer[100];
    char **strings;
    nptrs = backtrace(buffer, SIZE);
    printf("backtrace() returned %d addresses\n", nptrs);
    /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
    would produce similar output to the following: */
    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL)
    {
        perror("backtrace_symbols");
        exit(0);
    }
    for (j = 0; j < nptrs; j++)
    {
        printf("%s\n", strings[j]);
    }
    free(strings);
    exit(0);
    return;
}

void StartGame(void)
{
    STG_Init();
}

void ExitGame(RoundInfo * pRound)
{
    printf("My game over!\r\n");
    STG_Dispose();
    g_RunFlag = false;
}

int MyPlayerID = 0;

int GetMyPlayerID(void)
{
    return MyPlayerID;
}

int main(int argc, char* argv[])
{
    int ret = 0;

    if (argc == 7)
    {
        g_Action = argv[6];
    }
    else if (argc != 6 )
    {
        printf("Game args error!\r\n");
        return 0;
    }

    signal(SIGSEGV, OnSigSem);

    /* 提取命令行参数 */
    in_addr_t server_ip = inet_addr(argv[1]);
    in_port_t server_port = atoi(argv[2]);
    in_addr_t my_ip = inet_addr(argv[3]);
    in_port_t my_port = atoi(argv[4]);
    int my_id = atoi(argv[5]);
    char* my_name = strrchr(argv[0], '/');
    if (my_name == NULL)
    {
        my_name = argv[0];
    }
    else
    {
        my_name++;
    }

    /* 创建socket */
    m_socket_id = socket(AF_INET, SOCK_STREAM, 0);

    /* 绑定自己的IP */
    sockaddr_in my_addr;
    my_addr.sin_addr.s_addr = my_ip;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(my_port);

    long flag = 1;
    setsockopt(m_socket_id, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag));

    if (bind(m_socket_id, (sockaddr*)&my_addr, sizeof(sockaddr)) < 0)
    {
        printf("bind failed: %m\n");
        return -1;
    }

    /* 连接服务器 */
    sockaddr_in server_addr;
    server_addr.sin_addr.s_addr = server_ip;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    printf("my game try to connect server(%s:%u)\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    while(0 != connect(m_socket_id, (sockaddr*)&server_addr, sizeof(sockaddr)))
    {
        usleep(100*1000);
    };
    printf("my game connect server success\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    /* 向server注册 */
    char reg_msg[50]="";
    MyPlayerID = my_id;
    snprintf(reg_msg, sizeof(reg_msg) - 1, "reg: %d %s need_notify \n", my_id, my_name);
    send(m_socket_id, reg_msg, (int)strlen(reg_msg)+1, 0);

    //TRACE("%d\r\n", g_msg_queue.exit);

    StartGame();

    /* 开始游戏 */
    g_RunFlag = true;
    while(g_RunFlag)
    {
        char buffer[4096] = {0};
        int size = recv(m_socket_id, buffer, sizeof(buffer) - 1, 0);
        //int size = ReceiveMsg(buffer);
        if (size > 0)
        {
            //printf("[%s] %d\r\n", buffer, size);
            //MsgQueueAdd(buffer, size);
            TRACE("%s", buffer);
            if(server_msg_process(size, buffer) != true)
            {
                break;
            }
            //total_msg ++;
        }
    }

	close(m_socket_id);
    return 0;
}

