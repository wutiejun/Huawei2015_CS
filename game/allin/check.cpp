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

/* check机器人 */

int m_socket_id = -1;
const char *g_Action = NULL;

bool server_msg_process(int size, const char* msg)
{
    if (NULL != strstr(msg, "game-over"))
    {
        printf("Game over!\r\n");
        return false;
    }

    if (NULL != strstr(msg, "inquire/"))
    {
        const char* response = "check";
        if (g_Action != NULL)
        {
            response = g_Action;
        }
//        usleep(400 * 1000);
        send(m_socket_id, response, (int)strlen(response)+1, 0);
    }

    return true;
}

int main(int argc, char* argv[])
{
    int ret = 0;
    pthread_t subThread;

    if (argc == 7)
    {
        g_Action = argv[6];
    }
    else if (argc != 6 )
    {
        printf("Game args error!\r\n");
        return 0;
    }

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

    /* 开始游戏 */
    while(1)
    {
        char buffer[1024] = {'\0'};
        int size = recv(m_socket_id, buffer, sizeof(buffer) - 1, 0);
        if (size > 0)
        {
            if (server_msg_process(size, buffer))
            {
                continue;
            }
            else
            {
                break;
            }
        }
    }

	close(m_socket_id);

    return 0;
}

