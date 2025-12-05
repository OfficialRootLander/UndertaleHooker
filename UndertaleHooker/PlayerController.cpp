#include "pch.h"
#include <windows.h>
#include "PlayerController.h"
#include <thread>
#include <atomic>
#include "Globals.h"
#include <fstream>
#include "Draw.h"
#include "MinHook.h"
#include "Room.h"
#include <random>

OriginalFnType originalOnWorldSave = nullptr;
OriginalFnType originalOnSaveTick = nullptr;

OriginalFnType originalOnRoomLoadOrgin = nullptr;

OriginalFnType originalOnShopLoadOrgin = nullptr;


OriginalFnType originalOnSaveHookLoadOrgin = nullptr;

OriginalFnType originalOnInventoryHookLoadOrgin = nullptr;

OriginalFnType originalOnKeyLogged = nullptr;

OriginalFnType originalOnVerticalLoad = nullptr;

OriginalFnType orginalOnPlayerYHookOrgin = nullptr;

OriginalFnType orginalOnLongWindowHookOrgin = nullptr;

int PlayerControllerSaveCalls = 0; //later update pls
std::atomic<bool> IsInShop = false;
std::atomic<bool> MonitorRunning = true;
int ShopCounter = 0;
int ShopCheckCalls = 0;
int RoomID = 231;
int edi_value = 0;
int eax_value = 0;
int OnIncreaseValue = 0;

int SaveCalls = 0;
int InventoryCalls = 30;

uintptr_t OnDoubleIncreasePtr;

DWORD WINAPI ReEnableHookThread(LPVOID lpParam)
{
    Sleep(3000); // wait 3 seconds

    LPVOID target = lpParam; // function address to re-enable

    MH_EnableHook(target);

    printf("Re-enabled hook after 3 seconds!\n");
    return 0;
}


int __fastcall APlayerController::OnWorldSaveHook(APlayerController* thisPtr)
{
    PlayerControllerSaveCalls++;
    OnInteractionHook();
    //std::cout << "thisPtr: " << thisPtr << std::endl;
    if (originalOnWorldSave)
        return originalOnWorldSave(thisPtr);

    return 0;
}


const char* __fastcall APlayerController::SaveUndertaleHook(APlayerController* thisPtr)
{
    if (SaveCalls < 1200)
    {
        SaveCalls++;
        return "AAAAAA";
    }
    SaveCalls = 0;

    //feel free to add your own safe code here

    return "AAAAAA";
}


int __fastcall APlayerController::OnInventoryHook(APlayerController* thisPtr)
{
    if (InventoryCalls < 30)
    {
        InventoryCalls++;
        if (originalOnInventoryHookLoadOrgin)
            return originalOnInventoryHookLoadOrgin(thisPtr);
    }

    InventoryCalls = 0;

    std::cout << "\n[Inventory]: Inventory Hook called!";

    //const char* newItem = "FortniteDad";
    //memcpy((void*)0x28ED130, newItem, strlen(newItem) + 1);

    if (originalOnInventoryHookLoadOrgin)
        return originalOnInventoryHookLoadOrgin(thisPtr);
}

//first __cdecl
void* param = nullptr;
void* oldparam = nullptr;

int __fastcall APlayerController::OnRoomCallHook(APlayerController* thisPtr, void* secondParam)
{
    PlayerControllerSaveCalls++;
    uintptr_t addr = reinterpret_cast<uintptr_t>(secondParam);
   // std::cout << "SavTick... secondParam: " << std::hex << addr << std::endl;
    __asm
    {
        push edi
        mov edi_value, edi
        pop edi

        push eax
        mov eax_value, eax
        pop eax
    } 

    oldparam = param;          // Store current param as old
    param = secondParam;       // Update to new value

    if (originalOnSaveTick)
        return originalOnSaveTick(thisPtr); // still call original

    return 0;
}

void __fastcall APlayerController::OnInteractionHook()
{
    std::cout << "Interaction Hook Called!\n";

    //interaction code
}


//UNDERTALE.exe+1417D9 - DD 00                 - fld qword ptr [eax]

int __fastcall APlayerController::OnShopHook(APlayerController* thisPtr, void* secondParam)
{
    std::cout << "Shop Hook Called!\n";


    if (originalOnShopLoadOrgin)
        return originalOnShopLoadOrgin(thisPtr); // still call original

    return 0;

}

int MaxRooms = 220;
int __fastcall APlayerController::OnRoomLoadHook(APlayerController* thisPtr, void* secondParam)
{
    std::cout << "RoomLoad Hook Called!\n";

    __asm {
        push esi
        mov RoomID, esi
        pop esi
    }

    printf("RoomID: %d\n", RoomID);
    CurrentRoomID = RoomID; //hope this doesnt crash

    ShouldPairReset = true;

    if (originalOnRoomLoadOrgin)
        return originalOnRoomLoadOrgin(thisPtr); // still call original

    return 0;
}


void APlayerController::PrintNearbyValues(uintptr_t base, int startOffset, int endOffset)
{
    printf("Dumping values from offset 0x%X to 0x%X:\n", startOffset, endOffset);

    for (int offset = startOffset; offset <= endOffset; offset += 4) // stepping by 4 bytes (int/float size)
    {
        int* pInt = reinterpret_cast<int*>(base + offset);
        float* pFloat = reinterpret_cast<float*>(base + offset);

        // Print both int and float for inspection
        printf("Offset 0x%X: int = %d, float = %f\n", offset, *pInt, *pFloat);
    }
}

//UNDERTALE.exe+CB578 - FF 15 34836D00        - call dword ptr [UNDERTALE.exe+2D8334] //for setting long window call

int LocalWindowX = 0;

int __fastcall APlayerController::LongWindowHook(APlayerController* thisPtr)
{
    //std::cout << "LongWindowHookHasBeenCalled !!!!!!!!!!!!!!!!!!! \n";
    
    __asm
    {
        push edx
        mov edx, LocalWindowX
        pop edx
    }
    

    if (orginalOnLongWindowHookOrgin)
        return orginalOnLongWindowHookOrgin(thisPtr);
    return 0;
}


int __fastcall APlayerController::GetCurrentWindowHeight()
{
    uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));
    int* pXValue = reinterpret_cast<int*>(base + 0x3B7CCC);
    return *pXValue;
}

int __fastcall APlayerController::GetCurrentWindowWidth()
{
    uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));
    int* pXValue = reinterpret_cast<int*>(base + 0x3B7CC8);
    return *pXValue;
}
int __fastcall APlayerController::GetPlayerCords(bool X)
{
    uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));

    if (X)
    {
        // Directly read the int value at base + 0x4087F0
        int* pXValue = reinterpret_cast<int*>(base + 0x4087F0);
        return *pXValue;
    }
    else
    {
        // example for Y: assuming it's 4 bytes after X
        //PrintNearbyValues(base, 0x408700, 0x408B00);
        //0x4087D8
        int* pXValue = reinterpret_cast<int*>(base + 0x4087DC);
        return *pXValue;
    }
}

int LastY = 0;

int __fastcall APlayerController::OnPlayerVerticalHook(APlayerController* thisPtr)
{
    //get esi

    //int yVal = *(int*)((uintptr_t)thisPtr + 0xD4); // D4 offset from ESI previously
    //LastY = yVal;

    std::cout << "Received a Y of: " << LastY << "\n";

    if (originalOnVerticalLoad)
        return originalOnVerticalLoad(thisPtr);
    return 0;
}

int valueAtEsiD4 = 0;

int __fastcall APlayerController::OnPlayerYHook(APlayerController* thisPtr) //GetPlayerYHook Actually
{

    __asm {
        push eax
        push esi
        mov eax, [esi + 0xD4]
        mov valueAtEsiD4, eax
        pop esi
        pop eax
    }

    if (valueAtEsiD4 != 129 && valueAtEsiD4 != 138)
    {
        gPlayerY = valueAtEsiD4;
        //std::cout << "PlayerY: " << PlayerY << std::endl;
    }


    if (orginalOnPlayerYHookOrgin)
        return orginalOnPlayerYHookOrgin(thisPtr);

    return 0;
}

//00CC0214
//USER32.GetKeyState+51 - 75 3C                 - jne USER32.GetKeyState+8F for keystate


//129246A0 M walked address

//129F9E54
//Code :cvtsi2ss xmm0,[UNDERTALE.exe+4087F0]

//Code :cvtsi2ss xmm1,[UNDERTALE.exe+4087F0]

//Code :mov [UNDERTALE.exe+4087F0],esi
int PlayerCamX = 0;
int PlayerCamY = 0;

int MonitorTicks = 0;

bool RandomRooms = false;
bool LoadBeginRoom = true;
bool LoadedBeginRoom = false;

int RoomTicks = 0;

int WantedRoom = 68;

DWORD WINAPI APlayerController::PlayerMainThread(LPVOID lpParameter) //648 ends
{
    int LastRoomID = RoomID;
    while (MonitorRunning)
    {
        //here do everything in
        RoomTicks++;
        MonitorTicks++;
        if (MonitorTicks > 100)
        {
            PlayerCamX = GetPlayerCords(true);
            std::cout << "Given Cam PlayerX: " << PlayerCamX << "\n";

            PlayerCamY = gPlayerY;
            std::cout << "Given Cam PlayerY: " << PlayerCamY << "\n";

            MonitorTicks = 0;

            //some begin player functions
            if (!LoadedBeginRoom && LoadBeginRoom && RoomID > 3)
            {
                LoadedBeginRoom = true;
                Room::TeleportRoom(WantedRoom);
            }
        }

        if (LastRoomID != RoomID && RandomRooms)
        {
            RoomTicks = 0;
            if (RandomRooms)
            {
                if (LastRoomID != 0 && RoomID < MaxRooms && LastRoomID != 184 && LastRoomID != 108) //no mobs //some rooms you just cannot skip like 184 as last room, 108 also 
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200)); //safe

                    std::random_device rd; // seed
                    std::mt19937 gen(rd()); // mersenne twister engine
                    std::uniform_int_distribution<int> dist(4, MaxRooms + 3);

                    int RandomRoom = dist(gen);
                    Room::TeleportRoom(RandomRoom);
                }
            }
        }
        LastRoomID = RoomID;
        //int Y = GetPlayerCords(false);
        //std::cout << "\nGiven PlayerY: " << Y;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }


    return 0;
}

#include <windows.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

DWORD WINAPI APlayerController::SaveThread(LPVOID lpParameter)
{
    Sleep(10000);

    srand((unsigned)time(NULL)); // Seed random number generator

    std::string backupPath9 = "C:\\Users\\sammy\\AppData\\Local\\UNDERTALE\\Save Orginal\\file9";
    std::string originalPath9 = "C:\\Users\\sammy\\AppData\\Local\\UNDERTALE\\file9";

    std::string backupPath0 = "C:\\Users\\sammy\\AppData\\Local\\UNDERTALE\\Save Orginal\\file0";
    std::string originalPath0 = "C:\\Users\\sammy\\AppData\\Local\\UNDERTALE\\file0";

    // Helper lambda to copy and modify files
    auto copyAndModifyFile = [&](const std::string& backupPath, const std::string& originalPath)
        {
            std::ifstream inFile(backupPath);
            if (!inFile.is_open())
            {
                std::cout << "[!] Failed to open backup file " << backupPath << " for reading!\n";
                return false;
            }

            std::vector<std::string> lines;
            std::string line;
            while (std::getline(inFile, line))
            {
                lines.push_back(line);
            }
            inFile.close();

            if (lines.size() < 2)
            {
                std::cout << "[!] Backup file " << backupPath << " too short to modify second-to-last line!\n";
                return false;
            }

            int randomNumber = rand() % 300 + 70;
            lines[lines.size() - 2] = std::to_string(randomNumber);

            std::ofstream outFile(originalPath);
            if (!outFile.is_open())
            {
                std::cout << "[!] Failed to open original file " << originalPath << " for writing!\n";
                return false;
            }

            for (const auto& l : lines)
            {
                outFile << l << "\n";
            }
            outFile.close();

            std::cout << "[+] " << originalPath << " copied and modified successfully! Second-to-last line set to " << randomNumber << "\n";
            return true;
        };

    // Run for file9
    copyAndModifyFile(backupPath9, originalPath9);
    // Run for file0
    copyAndModifyFile(backupPath0, originalPath0);

    return 0;
}

int Key = 0;
int ESCCalls = 0;
