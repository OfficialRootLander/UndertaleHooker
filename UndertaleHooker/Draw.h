#pragma once
#ifndef DRAW_H
#define DRAW_H
#include <vector>

enum AllPlayerDrawModes
{
	FRISK,
	DOGBOAT,
	TORIEL,
	PAPYRUS,
	SANS
};

typedef struct
{
	float* X;
	float* Y;
	bool HasDecimal;
	int UpdateCounter;
	double SpriteID;
} CordsPair;

extern std::vector<CordsPair> AllPareFloats;


class UndertaleDrawer
{
public:
	static void GetPlayerCordsGlobal(int& X, int& Y);
	static int __fastcall UndertaleImageIDHook(UndertaleDrawer* ThisDrawer);
	static int __fastcall GeneralSpriteIDHook(UndertaleDrawer* ThisDrawer);
	static int __fastcall CordsMoveFuncHook(UndertaleDrawer* ThisDrawer);
	static DWORD WINAPI PlayerDrawThread(void* lParams);
	static void EmptyAllPares(); //for the X draw pairs
};

extern bool ShouldPairReset;

using DrawOriginalFnType = int(__fastcall*)(UndertaleDrawer*);
extern DrawOriginalFnType originalUndertaleSpriteIDHook;
extern DrawOriginalFnType originalUndertaleImageIDHook;
extern DrawOriginalFnType originalUndertaleCordsMoveFuncHook;

#endif