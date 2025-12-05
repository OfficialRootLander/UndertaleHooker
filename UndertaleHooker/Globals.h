#pragma once
#ifndef GLOBALS_H
#define GLOBALS_H
#include "windows.h"
#include "RootVector.h"
#include <vector>
#include <iostream>

extern uintptr_t hmodel_address;
extern bool RecentlyMoved;
extern int NeededMoveMentFramesForUpdate;
extern int CurrentMovemtnFrames;
extern int CurrentRoomID;

extern int CurrentPlayerPosCheckingFrames;
extern int CurrentPlayerPosNeededFrames;

extern std::vector<RootVector> AllPlayerrvs;
extern std::vector<RootVector> AllActorsrvs;
extern std::vector<int> AllActorIDSprites;

extern std::vector<RootVector> OldPlayerrvs;

extern int OldPlayerX;
extern int OldPlayerY;

extern int gPlayerY;

extern bool gConnectedTCP;

extern std::string GSip;
extern int GSport;


extern bool LocalConnected;

extern std::string gPlayerGlobalName;

extern int TicksNeededToForceUpdate;
extern int CurrentRoomTicks;

//Console related
extern bool gConsoleEnabled;
extern std::string gCurrentConsoleCommandStr;

#endif