#pragma once
#ifndef PLAYERSTATS_H
#define PLAYERSTATS_H
#include <iostream>

typedef struct
{
	double* PlayerStructBase;
	double* PlayerGold;
	double* PlayerHP;
	double* PlayerFillerHP; //UI fill HP
	double* PlayerLV;
	double* PlayerAttack;
	double* PlayerDefense;
	double* PlayerEX;
	double* CurrentMobID; //diabolical
} PlayerStats;

class APlayerStatistics
{
public:
	static double GetPlayerHP();
	static int __fastcall OnPlayerStructLoadHook(APlayerStatistics* thisPtr);
	static void SetMobID(double Value);
	static void InitPlayerStats();
	static DWORD WINAPI PlayerStatiscticsThread(LPVOID lParam);
};

using OriginalFnStatsType = int(__fastcall*)(APlayerStatistics*);
extern OriginalFnStatsType gOnPlayerStructOG;

#endif