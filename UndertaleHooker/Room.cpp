#include "pch.h"
#include "Room.h"
#include "Globals.h"

void Room::TeleportRoom(int roomID)
{

	//UNDERTALE.exe+C4075 - A1 E0CE7D00           - mov eax,[UNDERTALE.exe+3DCEE0]
	//UNDERTALE.exe+C4090 - 89 2D 848EA100        - mov [UNDERTALE.exe+618E84],ebp

	uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));
	int* RoomAAdjust = (int*)(uintptr_t)(base + 0x618E84);
	int* RoomBAdjust = (int*)(uintptr_t)(base + 0x3DCEE0);

	*RoomAAdjust = roomID;
	*RoomBAdjust = roomID;

	//after this set the new current room id since it doesnt go through roomhook
	CurrentRoomID = roomID;
}