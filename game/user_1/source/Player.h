#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>

#define GAME_OVER 0xffee

#define TRACE(format, args...) \
    do \
    {\
        printf("%s:%d:", __FILE__, __LINE__);\
        printf(format, ## args);\
    }while(0)

void SetPlayerId(int id);

#define WSADESCRIPTION_LEN 128
#define WSASYSSTATUS_LEN 128

typedef struct WSAData_ {
   int wVersion;
   int wHighVersion;
   char szDescription[WSADESCRIPTION_LEN+1];
   char szSystemStatus[WSASYSSTATUS_LEN+1];
   unsigned short iMaxSockets;
   unsigned short iMaxUdpDg;
   char * lpVendorInfo;
} WSADATA;

/*若比赛结束, 应返回GAME_OVER*/
//int RoundEntry(int& socket);
int GameOver(void);
void GameStart(void);
int GetPlayerId(void);
int PlayerMsgProc(char* recv_buf);
int RoundEntry(int& socket);

