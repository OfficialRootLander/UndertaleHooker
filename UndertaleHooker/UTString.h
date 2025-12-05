#pragma once
#ifndef UTSTRING_H
#define UTSTRING_H
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <vector>
#include <Windows.h>

struct TextReplacement
{
    const char* Target;       // what to search for
    const char* Replace;      // what to replace it with
    size_t TargetLength;      // length of Target string
    size_t ReplaceLength;     // length of Replace string
};

extern std::vector<TextReplacement> Replacements;


extern bool ShouldOverrideDialogue;

class UTString
{
public:
    static std::string GetUTString(std::string Str);
    static void OverrideStringNow(const char* ReplacementStr);
	static int __fastcall WriteStringHook(UTString* ThisPtr);
	static int __fastcall GetStringHookFunc(UTString* ThisPtr);
	static DWORD WINAPI StringThread(LPVOID lParams);
};

using OriginalUTStringFnType = int(__fastcall*)(UTString*);
extern OriginalUTStringFnType UTWriteStringHookOG;
extern OriginalUTStringFnType UTStringGetHookOG;

#endif