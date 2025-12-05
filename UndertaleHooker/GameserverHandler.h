#pragma once
#ifndef GAMESERVERHANDLER_H
#define GAMESERVERHANDLER_H

#include <winsock2.h>   // Add this for SOCKET type
#include <windows.h>    // Often needed alongside winsock2
#include "RootVector.h"
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include "PlayerController.h"
#include <sstream>
#include "Globals.h"

class GameserverHandler
{
public:
    static void UpdatePlayer(int x, int y, int RoomID, int Health, std::string username);
    static std::wstring RunPipe(const std::wstring& Ebuffer);
    static std::vector<RootVector> GetAllRoomIDPlayers(int RoomID, const std::string& localPlayerName);

    // TCP support
    static bool ConnectTcp(const char* ip, int port);
    static bool SendTcp(const std::wstring& msg);
    static std::wstring ReceiveTcp();
    static void CloseTcp();
    static void UpdatePlayerTcp(int x, int y, int RoomID, int Health, const std::string& username);
    static std::vector<RootVector> GetAllRoomIDPlayersTcp(int RoomID, const std::string& localPlayerName);
    static std::vector<RootVector> GetAllRoomIDPActorTcp(int RoomID, const std::string& localPlayerName);

    // Threaded multiplayer logic
    static void StartMultiplayerThread(const std::string& ip, int port);
    static void StopMultiplayerThread();
    static std::vector<RootVector> GetLatestRoomMates(); // thread-safe getter
    static std::vector<RootVector>GetLatestActorRoomMates();

private:
    static SOCKET TcpSocket;
    static std::thread MultiplayerThread;
    static std::atomic<bool> ThreadRunning;
    static std::mutex RoommateMutex;
    static std::vector<RootVector> LatestRoommates;
    static std::mutex ActorRoommateMutex;
    static std::vector<RootVector> LatestActorRoommates;
};

#endif
