#pragma once
#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H
#include <iostream>

class APlayerController
{
public:

	static int __fastcall OnWorldSaveHook(APlayerController* thisPtr);
	static int __fastcall OnRoomCallHook(APlayerController* thisPtr, void* edx);
	static void __fastcall OnInteractionHook();
	static int __fastcall OnShopHook(APlayerController* thisPtr, void* secondParam);
	static int __fastcall OnRoomLoadHook(APlayerController* thisPtr, void* secondParam);
	static const char* __fastcall SaveUndertaleHook(APlayerController* thisPtr);
	static int __fastcall OnInventoryHook(APlayerController* thisPtr);

	//Get methods
	static int __fastcall GetPlayerCords(bool X);

	//specific value methods
	static int __fastcall OnPlayerVerticalHook(APlayerController* thisPtr);

	//real y hook
	static int __fastcall OnPlayerYHook(APlayerController* thisPtr);

	//internal
	static void PrintNearbyValues(uintptr_t base, int startOffset, int endOffset);

	//window properties
	static int __fastcall GetCurrentWindowHeight();
	static int __fastcall GetCurrentWindowWidth();

	//Long Window Hook
	static int __fastcall LongWindowHook(APlayerController* thisPtr);

	//key presses

	static DWORD WINAPI PlayerMainThread(LPVOID lpParameter);
	static DWORD WINAPI SaveThread(LPVOID lpParameter);
};

extern int PlayerCamX;
extern int PlayerCamY;

extern int RoomTicks;
extern int RoomID;

using OriginalFnType = int(__fastcall*)(APlayerController*);
extern OriginalFnType originalOnWorldSave;
extern OriginalFnType originalOnSaveTick;
extern OriginalFnType originalOnRoomLoadOrgin;
extern OriginalFnType originalOnShopLoadOrgin;
extern OriginalFnType originalOnSaveHookLoadOrgin;
extern OriginalFnType originalOnInventoryHookLoadOrgin;
extern OriginalFnType originalOnKeyLogged;
extern OriginalFnType originalOnVerticalLoad;
extern OriginalFnType orginalOnPlayerYHookOrgin;
extern OriginalFnType orginalOnLongWindowHookOrgin;

#endif