#ifndef _MSG_PROC_H_
#define _MSG_PROC_H_

#include <string>

#define SEAT_MSG  "seat"
#define BLIND_MSG  "blind"
#define HOLD_MSG  "hold"
#define INQUIRE_MSG  "inquire"
#define FLOP_MSG  "flop"
#define TURN_MSG  "turn"
#define RIVER_MSG "river"
#define SHOWDOWN_MSG  "showdown"
#define POT_WIN_MSG  "pot-win"
#define GAME_OVER_MSG  "game-over"

#define CHECK_ACTION   "check"    //ÈÃÅÆ
#define CALL_ACTION    "call"     //¸ú×¢
#define RAISE_ACTION   "raise"    //¼Ó×¢
#define ALL_IN_ACTION  "all_in"   //ALL_IN
#define FOLD_ACTION    "fold"     //ÆúÅÆ

#define EOL "\n"
#define SPACE_CODE " \t\r\n"

int ProcSeatInfoMsg(std::string msg);
int ProcBlindMsg(std::string msg);
int ProcHoldCardMsg(std::string msg);
int ProcInquireMsg(std::string msg);
int ProcFolpMsg(std::string msg);
int ProcTurnMsg(std::string msg);
int ProcRiverMsg(std::string msg);
int ProcShowdownMsg(std::string msg);
int ProcPotWinMsg(std::string msg);
std::string trim(const std::string& str);
#endif