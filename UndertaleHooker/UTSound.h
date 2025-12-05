#pragma once
#ifndef UTSOUND_H
#define UTSOUND_H
#include <iostream>
#include <string.h>
#include <Windows.h>

class UTSound
{
public:
	static int __fastcall hkMusicCheck(UTSound* ThisPtr);
	static void StopSound();
	static void EnableSound();
	static void LockSound();
	static void PlaySound(std::string songoggtxt);
};

extern int IsMusicEnabled;

using OriginalUTSoundFnType = int(__fastcall*)(UTSound*);
extern OriginalUTSoundFnType hkMusicCheckOG;

#endif