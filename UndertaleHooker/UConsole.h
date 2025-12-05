#pragma once
#ifndef UCONSOLE_H
#define UCONSOLE_H
#include <iostream>

struct UCommand
{
	std::string HoleCommand;
	int CommandIndex;
	bool NeedsResponds;
	bool NeedsAdmin;

	UCommand(std::string EHoleCommand, int ECommandIndex, bool ENeedsResponds, bool ENeedsAdmin)
		: HoleCommand(EHoleCommand), CommandIndex(ECommandIndex), NeedsResponds(ENeedsResponds), NeedsAdmin(ENeedsAdmin)
	{
	}

	std::string GetHoleCommand() const
	{
		return HoleCommand;
	}

	int GetCommandIndex() const
	{
		return CommandIndex;
	}

	bool ResponseIsNeeded() const
	{
		return NeedsResponds;
	}

	bool AdminIsNeeded() const
	{
		return NeedsAdmin;
	}
};

class UConsole
{
public:
	static std::string ExecuteCommandFromStruct(UCommand UCommandStruct);
	static void ExecuteCommand(std::string UCommandStr);
	static UCommand GetCommandStruct(std::string CommandStr);
};

#endif