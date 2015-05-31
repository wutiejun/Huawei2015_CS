#include <stdio.h>
#include <stdlib.h>  //
#include <string.h>  //strlen
#include <unistd.h>  //usleep/close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "pthread.h"
#include <errno.h>
#include "Player.h"

int m_socket_id = -1;
int total_round = 0;
RoundInfo LocalRoundInfo = {0};

typedef struct msg_queue_entry_
{
    struct msg_queue_entry_ * pNextMsg;
    int MsgSize;
    void * pMsg;
} msg_queue_entry;

typedef struct msg_queue_
{
    int MsgCount;
    int exit;
    pthread_mutex_t Lock;
    msg_queue_entry * pFirstMsg;
    msg_queue_entry * pLastMsg;
} msg_queue;

msg_queue g_msg_queue;

void MsgQueueInit(void)
{
    memset(&g_msg_queue, 0, sizeof(0));
    pthread_mutex_init(&g_msg_queue.Lock, NULL);
    return;
}

void MsgQueueAdd(const char * pMsg, int size)
{
    msg_queue_entry *pNewMsg = NULL;
    do
    {
        pNewMsg = (msg_queue_entry *)malloc(sizeof(msg_queue_entry) + size);
        if (pNewMsg == NULL)
        {
            TRACE("%lu %d\r\n", sizeof(msg_queue_entry) + size, errno);
            usleep(1000);
        }
    } while (pNewMsg == NULL);

    while (g_msg_queue.MsgCount > 100)
    {
        TRACE("%d\r\n", g_msg_queue.MsgCount);
        usleep(1000);
    }

    memset(pNewMsg, 0, sizeof(msg_queue_entry) + size);

    pNewMsg->pMsg = pNewMsg + 1;
    pNewMsg->MsgSize = size;
    pNewMsg->pNextMsg = NULL;
    memcpy(pNewMsg->pMsg, pMsg, size);

    //printf("Add msg 0x%p %d;\r\n", pNewMsg, size);

    pthread_mutex_lock(&g_msg_queue.Lock);
    if (g_msg_queue.MsgCount == 0)
    {
        /* 两个必然同时为空 */
        g_msg_queue.pFirstMsg = pNewMsg;
    }
    else
    {
        g_msg_queue.pLastMsg->pNextMsg = pNewMsg;
    }
    g_msg_queue.pLastMsg = pNewMsg;
    g_msg_queue.MsgCount ++;
    pthread_mutex_unlock(&g_msg_queue.Lock);
    return;
}

msg_queue_entry * MsgQueueGet(void)
{
    msg_queue_entry *pMsg = NULL;
    pthread_mutex_lock(&g_msg_queue.Lock);
    if (g_msg_queue.MsgCount > 0)
    {
        pMsg = g_msg_queue.pFirstMsg;
        g_msg_queue.pFirstMsg = g_msg_queue.pFirstMsg->pNextMsg;
        if (g_msg_queue.pFirstMsg == NULL)
        {
            g_msg_queue.pLastMsg = NULL;
        }
        g_msg_queue.MsgCount --;
    }
    pthread_mutex_unlock(&g_msg_queue.Lock);
    return pMsg;
}


bool server_msg_process(int size, const char* msg)
{
    if (NULL != strstr(msg, "game-over"))
    {
        printf("Game over!\r\n");
        return false;
    }

    SER_MSG_TYPES Type = Msg_GetMsgType(msg, size);
    if (Type == SER_MSG_TYPE_seat_info)
    {
        total_round ++;
        printf("Start new round %d!\r\n", total_round);
        //Msg_SaveRoundMsg(&LocalRoundInfo);
        memset(&LocalRoundInfo, 0, sizeof(RoundInfo));
    }

    SER_MSG_TYPES msg_type = Msg_Read(msg, size, NULL, &LocalRoundInfo);

    //printf("Msg_Read:%d:%s\r\n", msg_type, Msg_GetMsgNameByType(msg_type));

    if (msg_type == SER_MSG_TYPE_inquire)
    {
        const char* response = "check";
        send(m_socket_id, response, sizeof("check"), 0);
    }

#if 0
    if (NULL != strstr(msg, "inquire/"))
    {
        //const char* response = "all_in";
        //SER_MSG_TYPES type = Msg_GetMsgType(msg, size);
        const char* response = "check";
        //printf("get msg %d: %s", (int)type, Msg_GetMsgNameByType(type));
        send(m_socket_id, response, (int)strlen(response)+1, 0);
    }
#endif

    return true;
}

void * MsgProcessThread(void *pArgs)
{
    msg_queue_entry * pMsg = NULL;
    int ret = 0;

    while (1)
    {
        pMsg = MsgQueueGet();
        if (pMsg == NULL)
        {
            usleep(1000);
            continue;
        }

        //printf("Get msg 0x%p %d;\r\n", pMsg, pMsg->MsgSize);

        ret = server_msg_process(pMsg->MsgSize, (const char *)pMsg->pMsg);
        free(pMsg);
        pMsg = NULL;
        if (ret == false)
        {
            break;
        }
    }

    g_msg_queue.exit = true;

    return NULL;
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

char TempBuffer[4096] = {0};
char TempBuffer2[4096] = {0};
int lastsize = 0;

/* 读取socket消息，解决消息包粘连问题 */
int ReceiveMsg(char buffer[1024])
{
    int RecSize = 0;
    int HasMsg = 0;
    int MsgEndIndex = 0;
    SER_MSG_TYPES msg_type = SER_MSG_TYPE_none;
    do
    {
        TRACE("Start rce msg:\r\n");
        RecSize = recv(m_socket_id, TempBuffer + lastsize, sizeof(TempBuffer) - 1 - lastsize, 0);
        msg_type = Msg_GetMsgType(TempBuffer, RecSize);
        MsgEndIndex = Msg_CheckMsgByType(TempBuffer, RecSize, msg_type);
        if (MsgEndIndex > 0)
        {
            HasMsg = 1;
            memcpy(buffer, TempBuffer, MsgEndIndex);
        }
        lastsize = RecSize - MsgEndIndex;
        if (lastsize > 0)
        {
            memset(TempBuffer2,0,sizeof(TempBuffer2));
            memcpy(TempBuffer2, TempBuffer + MsgEndIndex, lastsize);
            memset(TempBuffer,0,sizeof(TempBuffer));
            memcpy(TempBuffer, TempBuffer2, lastsize);
        }
    } while (HasMsg);
    printf("ReceiveMsg %d %d:\r\n[%s]", RecSize, MsgEndIndex, buffer);
    return MsgEndIndex;
}

int main(int argc, char* argv[])
{
    int ret = 0;
    pthread_t subThread;

    if(argc != 6)
    {
        return -1;
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
    printf("try to connect server(%s:%u)\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    while(0 != connect(m_socket_id, (sockaddr*)&server_addr, sizeof(sockaddr)))
    {
        usleep(100*1000);
    };
    printf("game connect server success\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    /* 向server注册 */
    char reg_msg[50]="";
    snprintf(reg_msg, sizeof(reg_msg) - 1, "reg: %d %s \n", my_id, my_name);
    send(m_socket_id, reg_msg, (int)strlen(reg_msg)+1, 0);

    /* 创建处理线程 */
    MsgQueueInit();

    ret = pthread_create(&subThread, NULL, MsgProcessThread, NULL);
    if (ret == -1)
    {
        printf("pthread_create error!%d \r\n", errno);
    }

    pthread_detach(subThread);

    g_msg_queue.exit = false;

    TRACE("%d\r\n", g_msg_queue.exit);

    /* 开始游戏 */
    while(g_msg_queue.exit == false)
    {
        char buffer[1024] = {'\0'};
        int size = recv(m_socket_id, buffer, sizeof(buffer) - 1, 0);
        //TRACE("%d\r\n", size);
        //int size = ReceiveMsg(buffer);
        if (size > 0)
        {
            MsgQueueAdd(buffer, size);
        }
    }

	close(m_socket_id);

    return 0;
}

