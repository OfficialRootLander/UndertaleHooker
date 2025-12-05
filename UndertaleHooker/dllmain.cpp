#include "pch.h"
#include <windows.h>
#include <stdio.h>
#include "PlayerController.h"
#include "PlayerStats.h"
#include "Draw.h"
#include "UTString.h"
#include "UTSound.h"
#include "GMLHook.h"
#include <thread>
#include "MinHook.h"
#pragma comment(lib, "libMinHook.x86.lib")
#include "Globals.h"
#include "DX9Hooker.h"
#include <fstream>
#include <string>
#include "Window.h"
#include "Test.h"
//libMinHook.x86
void LoadPlayerNameFromFile(const std::string& path)
{
    std::ifstream file(path);
    if (file.is_open())
    {
        std::getline(file, gPlayerGlobalName); // Reads the first line into the global string
        file.close();
    }
}


DWORD WINAPI MainThread(LPVOID lpParam) {
    //MessageBoxA(NULL, "Thread started!", "Debug", MB_OK);
    srand(static_cast<unsigned int>(time(NULL))); //for random
    // Allocate console
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    printf("DLL injected and running!\n");

    HMODULE hModuleBase = GetModuleHandle(NULL);
    uintptr_t hmodel_address = (uintptr_t)hModuleBase;
    printf("Base address of EXE: %p\n", hModuleBase);

    LoadPlayerNameFromFile("UTName.config");

    // Function addresses
    uintptr_t gamestepfunctionAddress = hmodel_address + 0x13E04E;
    uintptr_t SaveTickfunctionAddress = hmodel_address + 0xC3823;
    uintptr_t BuyfunctionAddress = hmodel_address + 0xC40E6;
    uintptr_t OnUndertaleSave = hmodel_address + 0x47CC5;
    uintptr_t OnInventoryOpen = hmodel_address + 0x1085FA;
    uintptr_t OnVerticalMovementAddress = hmodel_address + 0x4AF7;
    uintptr_t InventoryLoadfunction_address = hmodel_address + 0x3D3A5;
    uintptr_t UndertaleSpriteIDFunctionAddress = hmodel_address + 0x4B34;
    uintptr_t UndertaleCordinatesMoveFunc = hmodel_address + 0x3E21C;
    uintptr_t UndertaleGetStringHookFunc = hmodel_address + 0x1061A7;
    uintptr_t UndertaleWriteStringHookFunc = hmodel_address + 0x1061F5;
    uintptr_t UndertaleOnLoadPlayerStruct = hmodel_address + 0x1417D9;

    //UNDERTALE.exe+1F104A - 3B 4A 78              - cmp ecx,[edx+78]
    uintptr_t UndertaleSoundCheck = hmodel_address + 0x1F104A;

    // Start player main thread
    HANDLE hThread = CreateThread(nullptr, 0, APlayerController::PlayerMainThread, 0, 0, nullptr);
    if (hThread) CloseHandle(hThread);
    else MessageBoxA(NULL, "PlayerCreateThread failed!", "Error", MB_OK | MB_ICONERROR);

    // Initialize MinHook
    if (MH_Initialize() != MH_OK) {
        MessageBoxA(NULL, "Failed to initialize MinHook!", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    //------ Hooks with GMLHOOK::Hook ------


    //------ Hooks ------

    // APlayerController Hooks
    GMLHOOK::Hook(gamestepfunctionAddress, &APlayerController::OnWorldSaveHook, originalOnWorldSave, "OnWorldSave");
   GMLHOOK::Hook(SaveTickfunctionAddress, &APlayerController::OnRoomCallHook, originalOnSaveTick, "OnRoomCall");
    GMLHOOK::Hook(BuyfunctionAddress, &APlayerController::OnRoomLoadHook, originalOnRoomLoadOrgin, "OnRoomLoad");
    GMLHOOK::Hook(OnUndertaleSave, &APlayerController::SaveUndertaleHook, originalOnSaveHookLoadOrgin, "SaveUndertale");
   GMLHOOK::Hook(OnInventoryOpen, &APlayerController::OnInventoryHook, originalOnInventoryHookLoadOrgin, "OnInventoryOpen");
     GMLHOOK::Hook(InventoryLoadfunction_address, &APlayerController::OnPlayerYHook, orginalOnPlayerYHookOrgin, "OnPlayerY");

    // UndertaleDrawer Hooks
    GMLHOOK::Hook(UndertaleSpriteIDFunctionAddress, &UndertaleDrawer::GeneralSpriteIDHook, originalUndertaleSpriteIDHook, "GeneralSpriteIDHook");
    GMLHOOK::Hook(UndertaleCordinatesMoveFunc, &UndertaleDrawer::CordsMoveFuncHook, originalUndertaleCordsMoveFuncHook, "CordsMoveFuncHook");

    // UTString Hooks
    GMLHOOK::Hook(UndertaleGetStringHookFunc, &UTString::GetStringHookFunc, UTStringGetHookOG, "GetStringHook");
    GMLHOOK::Hook(UndertaleWriteStringHookFunc, &UTString::WriteStringHook, UTWriteStringHookOG, "WriteStringHook");

    // APlayerStatistics Hooks
   GMLHOOK::Hook(UndertaleOnLoadPlayerStruct, &APlayerStatistics::OnPlayerStructLoadHook, gOnPlayerStructOG, "OnLoadPlayerStruct");

    //UTSound Hooks
    GMLHOOK::Hook(UndertaleSoundCheck, &UTSound::hkMusicCheck, hkMusicCheckOG, "OnMusicCheckHook");

    static void* _defaultvoid = nullptr;



    std::cout << "Initialised All Client Hooks!\n";

    //connect to TCP server from the gs
    CreateThread(0, 0, UTString::StringThread, 0, 0, 0); //crashes on boss fights idk at the end of mobs
    CreateThread(0, 0, UndertaleDrawer::PlayerDrawThread, 0, 0, 0);
    //Statisctis thread
    CreateThread(0, 0, APlayerStatistics::PlayerStatiscticsThread, 0, 0, 0);
    //playeryhook

    //connect to TCP server from the gs
    InitDX9Hook(); // Only call this once

    //start window
    CreateThread(0, 0, RUNWINDOW, 0, 0, 0);

    // Keep thread alive so your hook stays active
    while (true) {
        Sleep(100);
    }

    // Free console when done
    FreeConsole();

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        HANDLE hThread = CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
        if (!hThread) {
            MessageBoxA(NULL, "CreateThread failed!", "Error", MB_OK | MB_ICONERROR);
        }
        else {
            CloseHandle(hThread);
        }
    }
    return TRUE;
}
