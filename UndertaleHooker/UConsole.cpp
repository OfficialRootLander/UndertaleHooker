#include "pch.h"
#include "UConsole.h"
#include <string>
#include <sstream> //for getting data out of commandStr
#include "GameserverHandler.h"
#include "Room.h"
#include "UTSound.h"
#include "PlayerStats.h"

std::string UConsole::ExecuteCommandFromStruct(UCommand UCommandStruct)
{
	if (UCommandStruct.GetCommandIndex() == 0)
	{
		return "Command is a null UCommand, can't proceed!";
	}
	else if (UCommandStruct.GetCommandIndex() == 1) //Exemple open 127.0.0.1
	{
		//open command, nothing needs to be send to gs
		std::string RespondsStr;
		std::string HoleCommand = UCommandStruct.GetHoleCommand();

		std::string chunk = HoleCommand;
		std::stringstream segmentStream(chunk);

		std::string openStr, ipStr;
		segmentStream >> openStr >> ipStr;

		RespondsStr = "Connecting to Gameserver with ip: " + ipStr;

		GameserverHandler::ConnectTcp(ipStr.c_str(), 7777);

		return RespondsStr;
	}
	else if (UCommandStruct.GetCommandIndex() == 2) //Exemple UNLOCK UTADMIN <code here>
	{
		//open command, nothing needs to be send to gs
		std::string RespondsStr;
		std::string HoleCommand = UCommandStruct.GetHoleCommand();

		std::string chunk = HoleCommand;
		std::stringstream segmentStream(chunk);

		std::string Start1Str, Start2Str, CodeStr;
		segmentStream >> Start1Str, Start2Str, CodeStr;

		//now send them as a full command to the server 
		std::wstring wcommand(HoleCommand.begin(), HoleCommand.end());
		GameserverHandler::SendTcp(wcommand);
		//get responds and more TODO
		RespondsStr = "Trying to get Admin Rights with code: " + CodeStr;

		return RespondsStr;
	}
	else if (UCommandStruct.GetCommandIndex() == 3) //Room teleport
	{
		std::string HoleCommand = UCommandStruct.GetHoleCommand();

		std::string chunk = HoleCommand;
		std::stringstream segmentStream(chunk);

		std::string Part1, ARoomID;
	    segmentStream >> Part1 >> ARoomID;

		std::cout << "Teleporting to Room with ID: " + ARoomID << "\n";

		Room::TeleportRoom(std::atoi(ARoomID.c_str()));
	}
	else if (UCommandStruct.GetCommandIndex() == 4) //Mob loader
	{
		std::string HoleCommand = UCommandStruct.GetHoleCommand();

		std::string chunk = HoleCommand;
		std::stringstream segmentStream(chunk);

		std::string Part1, AMobID;
		segmentStream >> Part1 >> AMobID;

		std::cout << "Teleporting MobID: " + AMobID << "\n";

		APlayerStatistics::SetMobID(std::atoi(AMobID.c_str()));
		Room::TeleportRoom(306);
	}
	else if (UCommandStruct.GetCommandIndex() == 5) //stop sound
	{
		UTSound::StopSound();
	}
	else if (UCommandStruct.GetCommandIndex() == 6) //enable sound
	{
		UTSound::EnableSound();
	}
	else if (UCommandStruct.GetCommandIndex() == 7) //lock sound
	{
		UTSound::LockSound();
	}

	return "Command Execution Succeded";
}

UCommand UConsole::GetCommandStruct(std::string CommandStr)
{
	//check with what it begins and based on that created a command struct
	if (CommandStr.rfind("OPEN", 0) == 0)
	{
		//usually for opening game servers, we dont have to send stuff to the gameserver at all
		UCommand GSOpenCommandStruct(CommandStr, 1, false, false);
		return GSOpenCommandStruct;
	}
	else if (CommandStr.rfind("UNLOCK UTADMIN", 0) == 0)
	{
		UCommand GSOpenCommandStruct(CommandStr, 2, false, true);
		return GSOpenCommandStruct;
	}
	else if (CommandStr.rfind("ROOM", 0) == 0)
	{
		UCommand GSOpenCommandStruct(CommandStr, 3, false, true);
		return GSOpenCommandStruct;
	}
	else if (CommandStr.rfind("MOB", 0) == 0)
	{
		UCommand GSOpenCommandStruct(CommandStr, 4, false, true);
		return GSOpenCommandStruct;
	}
	else if (CommandStr.rfind("S0", 0) == 0) //stop sound
	{
		UCommand GSOpenCommandStruct(CommandStr, 5, false, true);
		return GSOpenCommandStruct;
	}
	else if (CommandStr.rfind("S1", 0) == 0) //enable sound
	{
		UCommand GSOpenCommandStruct(CommandStr, 6, false, true);
		return GSOpenCommandStruct;
	}
	else if (CommandStr.rfind("S2", 0) == 0) //lock sound
	{
		UCommand GSOpenCommandStruct(CommandStr, 7, false, true);
		return GSOpenCommandStruct;
	}
	//default
	UCommand GSOpenCommandStruct(CommandStr, 0, false, false); //0 means invalid command
	return GSOpenCommandStruct;
}

void UConsole::ExecuteCommand(std::string CommandStr)
{
	//first Get the command struct
	UCommand CommandStruct = GetCommandStruct(CommandStr);
	std::string CommandResponds = ExecuteCommandFromStruct(CommandStruct);
	std::cout << "\n\n" << CommandResponds << "\n";
	//more checks here like message box
}
