#include "pch.h"
#include "UTSound.h"
#include "Room.h"
#include "PlayerController.h"


OriginalUTSoundFnType hkMusicCheckOG = nullptr;
//UNDERTALE.exe+1F104A - 3B 4A 78              - cmp ecx,[edx+78]
int edx_value = 0;
int IsMusicEnabled = 1;
int LastMusicInt = 1;
int old_edx_value = 0;

//UNDERTALE.exe+4C6B - F3 0F10 8E 84000000   - movss xmm1,[esi+00000084]
//turn above 1 and collision will turn off

//UNDERTALE.exe+4C6B - F3 0F10 8E 84000000   - movss xmm1,[esi+00000084] // X IS AT 80


int __fastcall UTSound::hkMusicCheck(UTSound* ThisPtr)
{
    if (LastMusicInt != IsMusicEnabled)
    {
        __asm {
            mov eax, [edx + 0x78] 
            mov edx_value, eax
        }
    }

    if (!IsMusicEnabled)
    {
        __asm {
            mov dword ptr[edx + 0x78], 0
        }
    }

    if (IsMusicEnabled == 2) //lock soundboard
    {
        __asm {
            mov eax, edx_value      
            mov dword ptr[edx + 0x78], eax 
        }
    }

    LastMusicInt = IsMusicEnabled;
    return hkMusicCheckOG(ThisPtr);
}


void UTSound::StopSound()
{
    std::cout << "Setted Music Off\n";
    IsMusicEnabled = 0; //simple as that
}

void UTSound::EnableSound()
{
    std::cout << "Setted Music On\n";
    IsMusicEnabled = 1;
}

void UTSound::LockSound()
{
    std::cout << "Locked Music\n";
    IsMusicEnabled = 2;
}

void UTSound::PlaySound(std::string songsoundtxt)
{

}