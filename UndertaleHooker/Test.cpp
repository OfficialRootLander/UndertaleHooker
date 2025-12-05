#include "pch.h"
#include "Test.h"


TestOriginalFnType originalTestSpriteDrawHook = nullptr;

void __fastcall Test::SpriteDrawHook(Test* ThisPtr) //this one is far better bet then original
{
	//here do shit

	//everything frisk for exemple
	*(int*)((uintptr_t)ThisPtr + 0x74) = 1131; 

	originalTestSpriteDrawHook(ThisPtr);
}