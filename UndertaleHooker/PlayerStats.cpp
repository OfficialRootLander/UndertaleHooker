#include "pch.h"
#include "PlayerStats.h"
#include <thread>
#include <atomic>

// full offset table relative to stable base 128F4640
/*
Address     Offset from 128F4640
128F4640    0x0
128F4660    0x20
128F4690    0x50
128F4710    0xD0
128F4750    0x110
128F49E0    0x380
128F4A50    0x3B0
128F4A60    0x3A0
128F4A70    0x3D0
128F4B00    0x4C0
128F4B30    0x4F0
128F4B40    0x500
128F4B50    0xF10
128F4AE0    0x3E0
128F4AF0    0x3F0
128F45E0    -0x80
128F45F0    -0x70
129EB5E0    0x1FAB0 - 0x128F4640 = 0x1FAB0
129EB5F0    0x1FAA0
129EB710    0x1FA40
129EBA60    0x1F590
129EBAE0    0x1F530
129EBAF0    0x1F520
129EBB00    0x1F500
129EB750    0x1FA60
*/


bool StatsThreadRunning = true;

OriginalFnStatsType gOnPlayerStructOG = nullptr;

uintptr_t PlayerDoubleUsedAddress = 0x0;

PlayerStats PlayerStatisctics = { 0 };

int __fastcall APlayerStatistics::OnPlayerStructLoadHook(APlayerStatistics* thisPtr)
{
    //        movsx ecx, byte ptr[eax]
    __asm
    {
        mov PlayerDoubleUsedAddress, eax
    }

    return gOnPlayerStructOG(thisPtr);
}

void OverrideWhenExists(double* AdjustAddress, double Variable)
{
    if (AdjustAddress != nullptr)
    {
        *AdjustAddress = Variable;
    }
}

double APlayerStatistics::GetPlayerHP() //pls make this a struct
{
    if (PlayerStatisctics.PlayerHP)
    {
        return *PlayerStatisctics.PlayerHP;
    }
    return 20;
}

void APlayerStatistics::SetMobID(double Value)
{
    OverrideWhenExists(PlayerStatisctics.CurrentMobID, Value);
}

void APlayerStatistics::InitPlayerStats() //normally we do this with a backend and shit
{
    //exemple infinite HP and attack
    OverrideWhenExists(PlayerStatisctics.PlayerAttack, 666666);
    OverrideWhenExists(PlayerStatisctics.PlayerHP, 666666);
    OverrideWhenExists(PlayerStatisctics.PlayerDefense, 666666);
    OverrideWhenExists(PlayerStatisctics.PlayerLV, 666666);
    OverrideWhenExists(PlayerStatisctics.CurrentMobID, 95); //sans
}

//Thread here

struct PlayerStatsLast
{
    double PlayerHP = -1;
    double PlayerGold = -1;
    double PlayerFillerHP = -1;
    double PlayerLV = -1;
    double PlayerAttack = -1;
    double PlayerDefense = -1;
    double PlayerEX = -1;
    double CurrentMobID = -1;
};

PlayerStatsLast LastPlayerStats;

DWORD WINAPI APlayerStatistics::PlayerStatiscticsThread(LPVOID lParam)
{
    while (StatsThreadRunning)
    {
        //std::cout << "Player Current Used Double: " << std::hex << PlayerDoubleUsedAddress << std::dec << "\n";

        if (PlayerDoubleUsedAddress != 0 && PlayerStatisctics.PlayerStructBase == nullptr) //only assign once
        {
            uintptr_t last3 = PlayerDoubleUsedAddress & 0xFFF;

            double valueAtAddr = *(double*)PlayerDoubleUsedAddress;

            if ((last3 == 0x640))
            {
                // Assign as base
                PlayerStatisctics.PlayerStructBase = (double*)PlayerDoubleUsedAddress;

                // Updated offsets relative to 128F4640

                PlayerStatisctics.PlayerHP = (double*)(PlayerDoubleUsedAddress + 0x500); //finished so far
                PlayerStatisctics.PlayerGold = (double*)(PlayerDoubleUsedAddress + 0x430);
                PlayerStatisctics.PlayerFillerHP = (double*)(PlayerDoubleUsedAddress + 0x4F0);
                PlayerStatisctics.PlayerLV = (double*)(PlayerDoubleUsedAddress + 0x410);
                PlayerStatisctics.PlayerAttack = (double*)(PlayerDoubleUsedAddress + 0x110);
                PlayerStatisctics.PlayerDefense = (double*)(PlayerDoubleUsedAddress + 0x4A0);
                PlayerStatisctics.PlayerEX = (double*)(PlayerDoubleUsedAddress + 0x420);
                PlayerStatisctics.CurrentMobID = (double*)(PlayerDoubleUsedAddress + 0x230);

                auto logStat = [](const char* name, double* addr)
                    {
                        if (addr)
                        {
                            __try
                            {
                                std::cout << name << " at: " << std::hex << addr
                                    << " With Value: " << std::dec << *addr << "\n";
                            }
                            __except (EXCEPTION_EXECUTE_HANDLER)
                            {
                                std::cout << name << " at: " << std::hex << addr
                                    << " is unreadable!\n";
                            }
                        }
                    };

                logStat("PlayerHP", PlayerStatisctics.PlayerHP);
                logStat("PlayerGold", PlayerStatisctics.PlayerGold);
                logStat("PlayerFillerHP", PlayerStatisctics.PlayerFillerHP);
                logStat("PlayerLV", PlayerStatisctics.PlayerLV);
                logStat("PlayerAttack", PlayerStatisctics.PlayerAttack);
                logStat("PlayerDefense", PlayerStatisctics.PlayerDefense);
                logStat("PlayerEX", PlayerStatisctics.PlayerEX);
                logStat("CurrentMobID", PlayerStatisctics.CurrentMobID);

                std::cout << "Player base struct assigned: " << std::hex << PlayerStatisctics.PlayerStructBase << std::dec << "\n";

                //12A1B870

                APlayerStatistics::InitPlayerStats();
            }
        }

        //this is where the fun begins
        auto tryRead = [](double* addr) -> double {
            __try { return *addr; }
            __except (EXCEPTION_EXECUTE_HANDLER) { return -1; }
            };

        // Only update/log if changed
        double hp = tryRead(PlayerStatisctics.PlayerHP);
        if (hp != -1 && hp != LastPlayerStats.PlayerHP)
        {
            std::cout << "PlayerHP changed: " << hp << "\n";
            LastPlayerStats.PlayerHP = hp;
        }

        double gold = tryRead(PlayerStatisctics.PlayerGold);
        if (gold != -1 && gold != LastPlayerStats.PlayerGold)
        {
            std::cout << "PlayerGold changed: " << gold << "\n";
            LastPlayerStats.PlayerGold = gold;
        }

        double fillerHP = tryRead(PlayerStatisctics.PlayerFillerHP);
        if (fillerHP != -1 && fillerHP != LastPlayerStats.PlayerFillerHP)
        {
            std::cout << "PlayerFillerHP changed: " << fillerHP << "\n";
            LastPlayerStats.PlayerFillerHP = fillerHP;
        }

        double lv = tryRead(PlayerStatisctics.PlayerLV);
        if (lv != -1 && lv != LastPlayerStats.PlayerLV)
        {
            std::cout << "PlayerLV changed: " << lv << "\n";
            LastPlayerStats.PlayerLV = lv;
        }

        double attack = tryRead(PlayerStatisctics.PlayerAttack);
        if (attack != -1 && attack != LastPlayerStats.PlayerAttack)
        {
            std::cout << "PlayerAttack changed: " << attack << "\n";
            LastPlayerStats.PlayerAttack = attack;
        }

        double def = tryRead(PlayerStatisctics.PlayerDefense);
        if (def != -1 && def != LastPlayerStats.PlayerDefense)
        {
            std::cout << "PlayerDefense changed: " << def << "\n";
            LastPlayerStats.PlayerDefense = def;
        }

        double ex = tryRead(PlayerStatisctics.PlayerEX);
        if (ex != -1 && ex != LastPlayerStats.PlayerEX)
        {
            std::cout << "PlayerEX changed: " << ex << "\n";
            LastPlayerStats.PlayerEX = ex;
        }

        double MobID = tryRead(PlayerStatisctics.CurrentMobID);
        if (MobID != -1 && MobID != LastPlayerStats.CurrentMobID)
        {
            std::cout << "CurrentMobID changed: " << MobID << "\n";
            LastPlayerStats.CurrentMobID = MobID;
        }


        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        //do more player stats 
    }
    return 0;
}