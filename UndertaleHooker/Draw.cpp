#include "pch.h"
#include "Draw.h"
#include "PlayerController.h"
#include <iostream>
#include <emmintrin.h> // SSE2
#include <fstream>
#include "MinHook.h"
#pragma comment(lib, "libMinHook.x86.lib")

#define MAX_PAIRS 1024

bool ShouldPairReset = false;

using DrawSpriteFn = void(__cdecl*)(int sprite_index, int image_index, float* x, float* y, void* context);

DrawSpriteFn draw_sprite_orig = nullptr;

struct DummyContext { char padding[0x40]; }; // make bigger just in case
DummyContext dummy_context;

// +DE090 draw sprite

typedef void(__cdecl* UTSpriteDraw_t)(
    int sprite_index,
    int image_index,
    float x,
    float y
    );



void UTSprite_Draw_Safe(int sprite_index, int image_index, float x, float y)
{
    static auto DrawSprite = (uintptr_t)GetModuleHandleA(NULL) + 0xDE090;
    DummyContext ctx;

    // push everything manually using __fastcall + inline assembly
    __asm {
        mov edx, sprite_index
        mov esi, ctx

        lea eax, x
        push eax
        lea ecx, y
        push ecx
        push esi
        push edx

        mov eax, DrawSprite
        call eax
    }
}


void DrawAllSpritesOnScreen()
{
    const int spriteWidth = 32;
    const int spriteHeight = 32;

    // screen size - adjust to Undertale's resolution
    const int screenWidth = 640;
    const int screenHeight = 480;

    int spriteIndex = 0;  // start at 0

    for (float y = 0; y < screenHeight; y += spriteHeight)
    {
        for (float x = 0; x < screenWidth; x += spriteWidth)
        {
            if (spriteIndex > 2000)
                return; // stop once we hit 2000

            UTSprite_Draw_Safe(spriteIndex, 1, x, y);
            spriteIndex++;
        }
    }
}

AllPlayerDrawModes DrawMode = TORIEL;

DrawOriginalFnType originalUndertaleImageIDHook = nullptr;
DrawOriginalFnType originalUndertaleSpriteIDHook = nullptr;
DrawOriginalFnType originalUndertaleCordsMoveFuncHook = nullptr;

uintptr_t PlayerCordsBase = 0;

double XMM0_SpriteValue = 0.0;
double XMM0_XMoveValue = 0.0;
int ECX_Object_Value = 0.0;

int GeneralLogTicks = 0;

static float* WriteObjectX[819200];
static float* WriteObjectY[819200];
static double* WriteObjectSprite[819200];
int CurrentObjectWriteInt = 0;

std::vector<CordsPair> AllPareFloats;

void UndertaleDrawer::EmptyAllPares()
{
    AllPareFloats.clear();
}

void AddPareFloat(float* OX, float* OY, double spriteID)
{
    bool Found = false;
    bool DecimalFloat = *OX != round(*OX);
    CordsPair ArgPair = { OX, OY, DecimalFloat, 0 , spriteID };

    for (auto& Pair : AllPareFloats)
    {
        if (OX == nullptr)
            continue;

        if (Pair.X == OX)
        {
            Found = true;

            if (*Pair.X != *ArgPair.X || *Pair.Y != *ArgPair.Y)
            {
                Pair.UpdateCounter++;
                std::cout << "UpdateCounter incremented! New value: " << Pair.UpdateCounter << "\n";
            }

            Pair.X = ArgPair.X;
            Pair.Y = ArgPair.Y;
            Pair.HasDecimal = DecimalFloat;
            Pair.SpriteID = spriteID;
            break;
        }
    }

    if (!Found)
    {
        if (AllPareFloats.size() >= MAX_PAIRS)
        {
            // remove oldest
            AllPareFloats.erase(AllPareFloats.begin());
        }
        AllPareFloats.push_back(ArgPair);
        std::cout << "New pair added: X=" << *OX << " Y=" << *OY << "\n";
    }
}


CordsPair* GetPlayerCords()
{
    CordsPair* bestCandidate = nullptr;
    int highestCounter = -1;

    for (auto& Pair : AllPareFloats)
    {
        if (Pair.X == nullptr)
            continue;
        // Filter: whole numbers, within reasonable game coordinates
        if (!Pair.HasDecimal && *Pair.X > 0 && *Pair.X < 2850.f && *Pair.Y > 0 && *Pair.Y < 600.f && !(*Pair.X == 0.f && *Pair.Y == 0.f))
        {
            // Pick the one with the highest update counter (most active object)
            if (Pair.UpdateCounter > highestCounter)
            {
                highestCounter = Pair.UpdateCounter;
                bestCandidate = &Pair;
            }
        }
    }

    return bestCandidate;
}

void UndertaleDrawer::GetPlayerCordsGlobal(int& X, int& Y)
{
    CordsPair* P = GetPlayerCords();
    if (!P)
    {
        X = 0;
        Y = 0;
    }
    else
    {
        X = *P->X;
        Y = *P->Y;
    }
}

void NegateAllBullets(CordsPair* PlayerCords)
{
    if (!PlayerCords) return;

    for (auto& Pair : AllPareFloats)
    {
        // skip the player itself
        if (*Pair.X == *PlayerCords->X && *Pair.Y == *PlayerCords->Y)
            continue;

        float dx = *Pair.X - *PlayerCords->X;
        float dy = *Pair.Y - *PlayerCords->Y;

        // only negate bullets close to the player
            //std::cout << "Negating Bullet! dx=" << dx << " dy=" << dy << "\n";

            // mirror bullet relative to player
       //first we need to actually find the bullets
        if(Pair.SpriteID == 108 || Pair.SpriteID == 109 || Pair.SpriteID == 106 && Pair.HasDecimal)
        {
            *Pair.X = -10000;
            *Pair.Y = -10000;
        }
    }
}



double AllSpriteIDs[] = { //idk about 1419 for papyrus left
    1618, 1191, 1200, 1195, 1196, 1412, 1414, 1417, 1419, 1443, 1452, 1453, 1457
};

enum AllSpriteID
{
    DOGBOAT_LEFT,
    TORIEL_DOWN,
    TORIEL_UP,
    TORIEL_RIGHT,
    TORIEL_LEFT,
    PAPYRUS_DOWN,
    PAPYRUS_UP,
    PAPYRUS_RIGHT,
    PAPYRUS_LEFT,
    SANS_DOWN,
    SANS_UP,
    SANS_RIGHT,
    SANS_LEFT
};

alignas(8) volatile double forcedSprite = 1131.0;
bool DrawImagesLoaded = true;

//PLS FIRST ENABLE IN MAIN I FORGOT THAT BUT IT BUGS, WE NEED TO DISABLE HOOK AFTER OVERWRITE
int __fastcall UndertaleDrawer::UndertaleImageIDHook(UndertaleDrawer* ThisDrawer) //counts for interactive objects all 
{
    if (!ThisDrawer) return originalUndertaleImageIDHook(ThisDrawer);

    // safe read
    int objectValue = *(int*)((uintptr_t)ThisDrawer + 0x74);
    ECX_Object_Value = objectValue;

    // optional: zero it for testing
    //here replace all shit

    if (RoomTicks > 500 && !DrawImagesLoaded)
    {
        DrawImagesLoaded = true;
        *(int*)((uintptr_t)ThisDrawer + 0x74) = 1400;
    }
    else
    {
        DrawImagesLoaded = false;
    }

    return originalUndertaleImageIDHook(ThisDrawer);
}


int __fastcall UndertaleDrawer::GeneralSpriteIDHook(UndertaleDrawer* ThisDrawer) //225 is a common enemy lol
{

    // Move the double into xmm0 safely
    __asm
    {
        movsd XMM0_SpriteValue, xmm0
    }

    if (XMM0_SpriteValue >= 1131 && XMM0_SpriteValue <= 1134)
    {
        __asm {
            movsd xmm0, forcedSprite
        }
    }

    // now call original function or trampoline
    return originalUndertaleSpriteIDHook(ThisDrawer);
}

DWORD WINAPI UndertaleDrawer::PlayerDrawThread(void* lParams) //has bugs but ok
{
    while (true)
    {
        GeneralLogTicks++;

        if (DrawMode == FRISK)
        {
            forcedSprite = XMM0_SpriteValue; //default
        }

        if (DrawMode == TORIEL)
        {
            switch ((int)XMM0_SpriteValue)
            {
            case 1131: forcedSprite = AllSpriteIDs[TORIEL_DOWN]; break;
            case 1132: forcedSprite = AllSpriteIDs[TORIEL_UP]; break;
            case 1133: forcedSprite = AllSpriteIDs[TORIEL_RIGHT]; break;
            case 1134: forcedSprite = AllSpriteIDs[TORIEL_LEFT]; break;
            default: forcedSprite = AllSpriteIDs[TORIEL_DOWN];
            }
        }
        
        if (DrawMode == PAPYRUS)
        {
            switch ((int)XMM0_SpriteValue)
            {
            case 1131: forcedSprite = AllSpriteIDs[PAPYRUS_DOWN]; break;
            case 1132: forcedSprite = AllSpriteIDs[PAPYRUS_UP]; break;
            case 1133: forcedSprite = AllSpriteIDs[PAPYRUS_RIGHT]; break;
            case 1134: forcedSprite = AllSpriteIDs[PAPYRUS_LEFT]; break;
            default: forcedSprite = AllSpriteIDs[PAPYRUS_DOWN];
            }
        }

        if (DrawMode == SANS)
        {
            switch ((int)XMM0_SpriteValue)
            {
            case 1131: forcedSprite = AllSpriteIDs[SANS_DOWN]; break;
            case 1132: forcedSprite = AllSpriteIDs[SANS_UP]; break;
            case 1133: forcedSprite = AllSpriteIDs[SANS_RIGHT]; break;
            case 1134: forcedSprite = AllSpriteIDs[SANS_LEFT]; break;
            default: forcedSprite = AllSpriteIDs[SANS_DOWN];
            }
        }

        if (DrawMode == DOGBOAT)
        {
            switch ((int)XMM0_SpriteValue)
            {
            case 1131: forcedSprite = AllSpriteIDs[DOGBOAT_LEFT]; break;
            case 1132: forcedSprite = AllSpriteIDs[DOGBOAT_LEFT]; break;
            case 1133: forcedSprite = AllSpriteIDs[DOGBOAT_LEFT]; break;
            case 1134: forcedSprite = AllSpriteIDs[DOGBOAT_LEFT]; break;
            default: forcedSprite = AllSpriteIDs[DOGBOAT_LEFT];
            }
        }


        if (ShouldPairReset)
        {
            UndertaleDrawer::EmptyAllPares();
            ShouldPairReset = false;
        }
        //get the player real x and y

        if (CurrentObjectWriteInt >= 8192)
            CurrentObjectWriteInt = 0; // or skip writing

        for (int i = 0; i < CurrentObjectWriteInt; i++)
        {
            if (WriteObjectX[i] != nullptr && WriteObjectY[i] != nullptr)
            {
                AddPareFloat(WriteObjectX[i], WriteObjectY[i], *WriteObjectSprite[i]); //dont add to much pare floats
            }
        }
        CurrentObjectWriteInt = 0;

        if (GeneralLogTicks > 60)
        {
            CordsPair* P = GetPlayerCords();
            if (P)
            {
                printf("Player Cords (%f,%f)\n", *P->X, *P->Y); //306 or 309 idk for default soul in box placement
                //UTSprite_Draw_Safe(1045, 1, 120.f, 100.f);
            }
            if (RoomID >= 200)
            {
                NegateAllBullets(P); //dont 
            }
            GeneralLogTicks = 0;
        }

        if (GetAsyncKeyState(VK_F1) & 0x1)
        {
            switch (DrawMode)
            {
            case FRISK: DrawMode = TORIEL; break;
            case TORIEL: DrawMode = PAPYRUS; break;
            case PAPYRUS: DrawMode = SANS; break;
            case SANS: DrawMode = DOGBOAT; break;
            case DOGBOAT: DrawMode = FRISK; break;
            }
        }

        Sleep(16);
    }

    return 0;
}


int __fastcall UndertaleDrawer::CordsMoveFuncHook(UndertaleDrawer* ThisDrawer) //this still needs bug fixing
{
    // Safe direct read
    float* pX = (float*)((uintptr_t)ThisDrawer + 0x9C);
    float* pY = (float*)((uintptr_t)ThisDrawer + 0xA0);
    //+ 0x74

    WriteObjectX[CurrentObjectWriteInt] = pX;
    WriteObjectY[CurrentObjectWriteInt] = pY;
    WriteObjectSprite[CurrentObjectWriteInt] = &XMM0_SpriteValue;
    CurrentObjectWriteInt++;  // simple atomic-safe increment

    //AddPareFloat(CurrentObjectX, CurrentObjectY);

    return originalUndertaleCordsMoveFuncHook(ThisDrawer);
}



/*

double forcedSprite = 1133.0;

int __fastcall UndertaleDrawer::GeneralSpriteIDHook(UndertaleDrawer* ThisDrawer)
{
    double spriteID = 1133.0;

    // Move the double into xmm0 safely
    __asm {
        movsd xmm0, spriteID
    }

    // now call original function or trampoline
    return originalUndertaleSpriteIDHook(ThisDrawer);
}


*/