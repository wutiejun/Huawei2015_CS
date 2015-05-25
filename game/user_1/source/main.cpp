#include <stdio.h>
//#include <WinSock2.h>
#include <assert.h>
#include <time.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Player.h"

char player_name[32] = "2026";      //请更改队名
char server_ip[32] = "127.0.0.1";
unsigned long server_port = 6000;

void ParseCmdPara(int argc, const char * argv[],unsigned long* player_id, unsigned long* server_port_no,char* server_ip)
{
    //第一个参数必须为server ip（可选）
    if(argc > 1)
    {
        strncpy(server_ip, argv[1], 32);
    }

    //第二个参数必须为server端口号（可选）
    if(argc > 2)
    {
        *server_port_no = atol(argv[2]);
    }
    
    unsigned long exenamelen = (unsigned long)strlen(argv[0]);
    const char* ch = NULL;
    for(ch=argv[0] + exenamelen; ch!=argv[0];ch--)
    {
        if(*ch == '\\')
        {
            ch++;
            break;
        }
    }
    
    *player_id=atol(ch);    
}

int main(int argc, const char * argv[])
{
    int w_version_requested;
    WSADATA ws_adata;
    int err;
    unsigned long player_id = 0;

    ParseCmdPara(argc, argv, &player_id, &server_port, server_ip);

#if 0
    ANSItoUTF8(player_name);

    srand( (unsigned)time( NULL ) * player_id);

	SetPlayerId(player_id);

    w_version_requested = MAKEWORD(1,1);
    err = WSAStartup(w_version_requested,&ws_adata);

    if(0 != err)
    {
        return;
    }

    if((LOBYTE(ws_adata.wVersion)!=1)||

        (HIBYTE(ws_adata.wVersion)!=1))
    {
        WSACleanup();
        return;
    }
#endif    

    GameStart();

    int sock_client = socket(AF_INET,SOCK_STREAM, 0);    
    struct sockaddr_in addr_srv;
    
    addr_srv.sin_family = AF_INET;
    addr_srv.sin_port = htons((unsigned short)server_port);
    addr_srv.sin_addr.s_addr = inet_addr(server_ip);    

//    stAdd.sin_family = AF_INET;
//    stAdd.sin_port = g_EchoServer.stArg.mPort;
//    stAdd.sin_addr.s_addr = g_EchoServer.stArg.mAddr;
    
    while(0 != connect(sock_client,(struct sockaddr*)&addr_srv, sizeof(struct sockaddr)))
    {
        sleep(10);
    };
    
    char hello[64]={0};
    
    sprintf(hello, "reg: %d %s \n", player_id, player_name);
    
    send(sock_client,hello, (int)strlen(hello)+1, 0);

    while(1)
    {
        //printf("\n-------round--------\n");
        if(GAME_OVER == RoundEntry(sock_client))
        {
            break;
        }
    }

	GameOver();
    shutdown(sock_client, SHUT_RDWR);
	close(sock_client);
	//WSACleanup();
    return 0;
}