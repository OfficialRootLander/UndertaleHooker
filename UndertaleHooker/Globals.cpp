#include "pch.h"
#include "Globals.h"

uintptr_t hmodel_address = 0x0;
bool RecentlyMoved = true;
int NeededMoveMentFramesForUpdate = 15;
int CurrentMovemtnFrames = 0;
int CurrentRoomID = 68; //for now
int CurrentPlayerPosCheckingFrames = 0;
int CurrentPlayerPosNeededFrames = 1;

std::vector<RootVector> AllPlayerrvs;

std::vector<RootVector> AllActorsrvs;
std::vector<int> AllActorIDSprites;

std::vector<RootVector> OldPlayerrvs;

int OldPlayerX = 0;
int OldPlayerY = 0;

int gPlayerY = 0;

bool gConnectedTCP = false;

std::string GSip = "26.231.213.89";
int GSport = 7777;

bool LocalConnected = false;

std::string gPlayerGlobalName = "MaceWindu";

int TicksNeededToForceUpdate = 30;
int CurrentRoomTicks = 0;

//Console related
extern bool gConsoleEnabled = false;
extern std::string gCurrentConsoleCommandStr = "";

