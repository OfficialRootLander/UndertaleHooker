#pragma once
#ifndef TEST_H
#define TEST_H
#include "Windows.h"
#include "Globals.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;
#define HMODULE_ADDRESS ((uintptr_t)&__ImageBase)

static void(*draw_sprite)(int, int, float, float);

class Test
{
public:

	///UNDERTALE.exe+40F47 - E8 84710500           - call UNDERTALE.exe+980D0


	// in .cpp


	static void __fastcall SpriteDrawHook(Test* ThisPtr);
};

using TestOriginalFnType = int(__fastcall*)(Test*);
extern TestOriginalFnType originalTestSpriteDrawHook;

#endif