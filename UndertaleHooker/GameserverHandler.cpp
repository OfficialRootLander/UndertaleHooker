#include "pch.h"
#include <winsock2.h> // Windows Sockets API (for TCP)
#include <windows.h>
#include <iostream>
#include <string>
#include "GameServerHandler.h"

#define PIPE_NAME LR"(\\.\pipe\UndernetGS)"
#include <iostream>
#include <string>

#include <ws2tcpip.h> // For sockaddr_in and related networking stuff

#pragma comment(lib, "ws2_32.lib")

SOCKET GameserverHandler::TcpSocket = INVALID_SOCKET;


std::string AsciiToHex(const std::string& input)
{
    std::ostringstream oss;
    oss << std::hex << std::uppercase;

    for (unsigned char c : input)
    {
        oss.width(2);
        oss.fill('0');
        oss << static_cast<int>(c);
    }

    return oss.str();
}

bool IsAPipeError(std::string response)
{
    //TODO update this
    return false;
}

std::string ConvertWStrToString(std::wstring wstring)
{
    size_t len = wstring.length() + 1;
    char* buffer = new char[len];
    size_t converter = 0;
    wcstombs_s(&converter, buffer, len, wstring.c_str(), len - 1);
    std::string str(buffer);
    return str;
}

bool GameserverHandler::ConnectTcp(const char* ip, int port)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    TcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (TcpSocket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;

#pragma warning(push)
#pragma warning(disable : 4996) // Disable warning C4996: 'inet_addr': deprecated

    serverAddr.sin_addr.s_addr = inet_addr(ip);

#pragma warning(pop)

    serverAddr.sin_port = htons(port);

    if (connect(TcpSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Connect failed\n";
        closesocket(TcpSocket);
        WSACleanup();
        return false;
    }

    return true;
}

bool GameserverHandler::SendTcp(const std::wstring& msg)
{
    std::cout << "[TCP] TcpSocket value: " << TcpSocket << std::endl;
    if (TcpSocket == INVALID_SOCKET) return false;

    int sent = send(TcpSocket,
        reinterpret_cast<const char*>(msg.c_str()),
        static_cast<int>((msg.size() + 1) * sizeof(wchar_t)), 0);

    if (sent == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        std::cerr << "[TCP] Send failed: " << err << "\n";
        return false;
    }
    return true;
}

std::wstring GameserverHandler::ReceiveTcp()
{
    if (TcpSocket == INVALID_SOCKET) return L"";

    wchar_t buffer[512] = {};
    int received = recv(TcpSocket,
        reinterpret_cast<char*>(buffer),
        sizeof(buffer) - sizeof(wchar_t), 0);

    if (received > 0)
    {
        buffer[received / sizeof(wchar_t)] = L'\0';
        return std::wstring(buffer);
    }
    else
    {
        std::cerr << "Receive failed or connection closed\n";
        return L"";
    }
}

void GameserverHandler::CloseTcp()
{
    if (TcpSocket != INVALID_SOCKET)
    {
        closesocket(TcpSocket);
        TcpSocket = INVALID_SOCKET;
    }
    WSACleanup();
}


void GameserverHandler::UpdatePlayerTcp(int x, int y, int RoomID, int Health, const std::string& username)
{
    std::string command = "UPDATE PLAYER " + std::to_string(x) + " " + std::to_string(y) + " " +
        std::to_string(RoomID) + " " + std::to_string(Health) + " " + username;

    std::wstring wcommand(command.begin(), command.end());
    std::wcout << L"Updated PlayerData WString (TCP): " << wcommand << L"\n";

    GameserverHandler::SendTcp(wcommand);
    GameserverHandler::ReceiveTcp(); // optional: ignore or log the response
}

std::vector<RootVector> GameserverHandler::GetAllRoomIDPlayersTcp(int RoomID, const std::string& localPlayerName)
{
    std::vector<RootVector> RoomMates;

    std::string command = "GET ROOMMATES " + std::to_string(RoomID);
    std::wstring wcommand(command.begin(), command.end());
    std::wcout << L"[TCP] Request: " << wcommand << L"\n";

    if (!GameserverHandler::SendTcp(wcommand))
    {
        std::wcout << L"[TCP] Failed to send GET ROOMMATES request.\n";
        return RoomMates;
    }

    std::wstring responseW = GameserverHandler::ReceiveTcp();
    std::string response(ConvertWStrToString(responseW));

    if (IsAPipeError(response)) // reused pipe error check — generic enough
    {
        std::wcout << L"[TCP] Error received, exiting GetAllRoomIDPlayersTcp\n";
        return RoomMates;
    }

    // Parse like before
    size_t start = 0;
    size_t end = response.find("||");

    while (end != std::string::npos)
    {
        std::string chunk = response.substr(start, end - start);
        std::stringstream segmentStream(chunk);

        int x, y;
        std::string name;
        segmentStream >> x >> y >> name;

        if (!segmentStream.fail() && name != localPlayerName)
        {
            RoomMates.emplace_back(x, y);
        }

        start = end + 2;
        end = response.find("||", start);
    }

    // Final trailing data
    if (start < response.length())
    {
        std::string chunk = response.substr(start);
        std::stringstream segmentStream(chunk);

        int x, y;
        std::string name;
        segmentStream >> x >> y >> name;

        if (!segmentStream.fail() && name != localPlayerName)
        {
            RoomMates.emplace_back(x, y);
        }
    }

    return RoomMates;
}

std::vector<RootVector> GameserverHandler::GetAllRoomIDPActorTcp(int RoomID, const std::string& localPlayerName)
{
    std::vector<RootVector> RoomMates;

    std::string command = "GET ROOMACTORS " + std::to_string(RoomID);
    std::wstring wcommand(command.begin(), command.end());
    std::wcout << L"[TCP] Request: " << wcommand << L"\n\n\n\n\n\n\n\n\n\n";

    if (!GameserverHandler::SendTcp(wcommand))
    {
        std::wcout << L"[TCP] Failed to send GET ROOMACTORS request.\n";
        return RoomMates;
    }

    std::wstring responseW = GameserverHandler::ReceiveTcp();
    std::string response(ConvertWStrToString(responseW));

    if (IsAPipeError(response)) // reused pipe error check — generic enough
    {
        std::wcout << L"[TCP] Error received, exiting GetAllRoomIDPlayersTcp\n";
        return RoomMates;
    }

    // Parse like before
    size_t start = 0;
    size_t end = response.find("||");

    while (end != std::string::npos)
    {
        std::string chunk = response.substr(start, end - start);
        std::stringstream segmentStream(chunk);

        int x, y, spriteID;
        std::string name;
        segmentStream >> x >> y >> name >> spriteID;

        if (!segmentStream.fail())
        {
            RoomMates.emplace_back(x, y);
        }

        //also place the spriteid in all roommates
        AllActorIDSprites.emplace_back(spriteID);

        start = end + 2;
        end = response.find("||", start);
    }

    // Final trailing data
    if (start < response.length())
    {
        std::string chunk = response.substr(start);
        std::stringstream segmentStream(chunk);

        int x, y;
        std::string name;
        segmentStream >> x >> y >> name;

        if (!segmentStream.fail() && name != localPlayerName)
        {
            RoomMates.emplace_back(x, y);
        }
    }

    return RoomMates;
}


void GameserverHandler::UpdatePlayer(int x, int y, int RoomID, int Health, std::string username)
{
    std::string Commandheader = "UPDATE PLAYER";
    std::string Strcommand = Commandheader + " " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(RoomID) + " " + std::to_string(Health) + " " + username; //UPDATE PLAYER 100 100 100 100 Cryonix
    std::wcout << L"Updated PlayerData WString: " + std::wstring(Strcommand.begin(), Strcommand.end()) + L"\n";
    GameserverHandler::RunPipe(std::wstring(Strcommand.begin(), Strcommand.end()));
}

std::vector<RootVector> GameserverHandler::GetAllRoomIDPlayers(int RoomID, const std::string& localPlayerName)
{
    std::vector<RootVector> RoomMates;

    std::string CommandHeader = "GET ROOMMATES";
    std::string Strcommand = CommandHeader + " " + std::to_string(RoomID);
    std::wcout << L"GET ROOMMATES WString: " + std::wstring(Strcommand.begin(), Strcommand.end()) + L"\n";

    std::wstring response = GameserverHandler::RunPipe(std::wstring(Strcommand.begin(), Strcommand.end()));
    std::string Strresponse = ConvertWStrToString(response);

    if (IsAPipeError(Strresponse))
    {
        std::wcout << L"A Pipe Error occurred while retrieving all roommates, returning...\n";
        return RoomMates;
    }

    // Split by "||"
    size_t start = 0;
    size_t end = Strresponse.find("||");

    while (end != std::string::npos)
    {
        std::string chunk = Strresponse.substr(start, end - start);

        std::stringstream segmentStream(chunk);
        int x, y;
        std::string name;
        segmentStream >> x >> y >> name;

        if (!segmentStream.fail())
        {
            if (name != localPlayerName)  // filter out local player
            {
                RoomMates.emplace_back(x, y);
            }
        }

        start = end + 2; // move past the "||"
        end = Strresponse.find("||", start);
    }

    // Handle any trailing data after the last "||"
    if (start < Strresponse.length())
    {
        std::string chunk = Strresponse.substr(start);
        std::stringstream segmentStream(chunk);
        int x, y;
        std::string name;
        segmentStream >> x >> y >> name;

        if (!segmentStream.fail())
        {
            if (name != localPlayerName)
            {
                RoomMates.emplace_back(x, y);
            }
        }
    }

    return RoomMates;
}


std::wstring GameserverHandler::RunPipe(const std::wstring& Ebuffer)
{
    // Try to open the pipe
    HANDLE hPipe = CreateFileW(
        PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        std::wcerr << L"Failed to connect to pipe. Error: " << GetLastError() << std::endl;
        return L"CONNECT ERROR";
    }

    // Command to send to server
    std::wstring command = Ebuffer;

    DWORD bytesWritten = 0;
    BOOL success = WriteFile(
        hPipe,
        command.c_str(),
        (DWORD)((command.size() + 1) * sizeof(wchar_t)),  // include null terminator
        &bytesWritten,
        NULL);

    if (!success)
    {
        std::wcerr << L"Failed to write to pipe. Error: " << GetLastError() << std::endl;
        CloseHandle(hPipe);
        return L"WRITE ERROR";
    }

    // Buffer for response
    wchar_t buffer[512];
    DWORD bytesRead = 0;

    success = ReadFile(
        hPipe,
        buffer,
        sizeof(buffer) - sizeof(wchar_t),
        &bytesRead,
        NULL);

    if (!success || bytesRead == 0)
    {
        std::wcerr << L"Failed to read from pipe. Error: " << GetLastError() << std::endl;
        CloseHandle(hPipe);
        return L"READ ERROR";
    }

    // Null-terminate the received string
    buffer[bytesRead / sizeof(wchar_t)] = L'\0';

    std::wcout << L"Response from server: " << buffer << std::endl;

    return buffer;
    CloseHandle(hPipe);
    return 0;
}

std::thread GameserverHandler::MultiplayerThread;
std::atomic<bool> GameserverHandler::ThreadRunning{ false };


std::mutex GameserverHandler::RoommateMutex;
std::vector<RootVector> GameserverHandler::LatestRoommates;

std::mutex GameserverHandler::ActorRoommateMutex;
std::vector<RootVector> GameserverHandler::LatestActorRoommates;

int CallsNeededToUpdateActorList = 60;
int CurrentActorCalls = 0;

void GameserverHandler::StartMultiplayerThread(const std::string& ip, int port)
{
    if (ThreadRunning) return;

    ThreadRunning = true;
    MultiplayerThread = std::thread([ip, port]()
        {
            if (!ConnectTcp(ip.c_str(), port))
            {
                std::cerr << "[TCP] Failed to connect in thread\n";
                ThreadRunning = false;
                return;
            }

            while (ThreadRunning)
            {
                int myX = APlayerController::GetPlayerCords(true);
                int myY = gPlayerY;

                UpdatePlayerTcp(myX, myY, CurrentRoomID, 20, gPlayerGlobalName);
                auto players = GetAllRoomIDPlayersTcp(CurrentRoomID, gPlayerGlobalName);

                {
                    std::lock_guard<std::mutex> lock(RoommateMutex);
                    LatestRoommates = players;
                }

                CurrentActorCalls++;
                if (CurrentActorCalls >= CallsNeededToUpdateActorList)
                {
                    auto actors = GetAllRoomIDPActorTcp(CurrentRoomID, gPlayerGlobalName);
                    {
                        std::lock_guard<std::mutex> lock(ActorRoommateMutex);
                        LatestActorRoommates = actors;
                    }
                    CurrentActorCalls = 0;
                }

                Sleep(33); // ~30 FPS
            }

            CloseTcp();
        });
}

void GameserverHandler::StopMultiplayerThread()
{
    if (!ThreadRunning) return;

    ThreadRunning = false;
    if (MultiplayerThread.joinable())
        MultiplayerThread.join();
}

std::vector<RootVector> GameserverHandler::GetLatestRoomMates()
{
    std::lock_guard<std::mutex> lock(RoommateMutex);
    return LatestRoommates;
}

std::vector<RootVector> GameserverHandler::GetLatestActorRoomMates()
{
    std::lock_guard<std::mutex> lock(ActorRoommateMutex);
    return LatestActorRoommates;
}

