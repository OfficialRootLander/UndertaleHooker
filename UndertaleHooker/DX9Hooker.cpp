#include "pch.h"
#include "DX9Hooker.h"
#include <Windows.h>
#include <cstdio>
#include "MinHook.h"
#include "d3dx9.h" // For texture loading and sprite interface
#include "PlayerController.h"
#include "Globals.h"
#include "GameserverHandler.h"
#include "Textures.h"
#include "UConsole.h" //for the console string handler

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib") // Make sure to link d3dx9.lib for texture loading

typedef HRESULT(__stdcall* EndScene_t)(LPDIRECT3DDEVICE9);
static EndScene_t oEndScene = nullptr;

static bool hookInstalled = false;
static IDirect3DTexture9* g_pFriskTexture = nullptr;

static IDirect3DTexture9* g_pActorPlaceHolderTexture = nullptr;

static IDirect3DTexture9* g_pMainCharaTexture = nullptr;

static IDirect3DTexture9* g_pConsoleTexture = nullptr;

static int RecentPlayerX;


ID3DXFont* g_pFont = nullptr;

void DrawTextOnScreen(const std::string& text, int x, int y, D3DCOLOR color = D3DCOLOR_XRGB(255, 255, 255)) { //From Stackoverflow, yes I dont know exactly how to use DirectX
    if (!g_pFont) return;

    RECT rect;
    SetRect(&rect, x, y, x + 800, y + 30); // Adjust width/height for your UI

    g_pFont->DrawTextA(
        nullptr,         // No sprite
        text.c_str(),    // The text to draw
        -1,              // Length (-1 means null-terminated)
        &rect,           // Rectangle for text
        DT_LEFT | DT_TOP,// Alignment
        color            // Color
    );
}


void InitFont(LPDIRECT3DDEVICE9 pDevice) {
    D3DXCreateFont(
        pDevice,
        18,                  // Height (font size)
        0,                   // Width (0 = auto)
        FW_NORMAL,           // Weight
        1,                   // Mip levels
        FALSE,               // Italic
        DEFAULT_CHARSET,     // Charset
        OUT_DEFAULT_PRECIS,  // Output precision
        ANTIALIASED_QUALITY, // Quality
        DEFAULT_PITCH | FF_DONTCARE, // Pitch and family
        L"Arial",            // Font face
        &g_pFont             // Output
    );
}

void PollConsoleInput() {
    for (char c = 'A'; c <= 'Z'; ++c) {
        if (GetAsyncKeyState(c) & 0x1) { // 0x1 = Key pressed this frame
            gCurrentConsoleCommandStr.push_back(c);
        }
    }
    for (char c = '0'; c <= '9'; ++c) {
        if (GetAsyncKeyState(c) & 0x1) {
            gCurrentConsoleCommandStr.push_back(c);
        }
    }

    //dot
    if (GetAsyncKeyState(VK_OEM_PERIOD) & 0x1)
    {
        gCurrentConsoleCommandStr.push_back('.');
    }

    // Space
    if (GetAsyncKeyState(VK_SPACE) & 0x1) {
        gCurrentConsoleCommandStr.push_back(' ');
    }
    // Backspace
    if (GetAsyncKeyState(VK_BACK) & 0x1) {
        if (!gCurrentConsoleCommandStr.empty())
            gCurrentConsoleCommandStr.pop_back();
    }
}


IDirect3DTexture9* GetDirectXActorTexture(int SpriteID)
{
    if (SpriteID == 1)
    {
        return g_pActorPlaceHolderTexture;
    }
    else if (SpriteID == 2)
    {
        return g_pMainCharaTexture;
    }
    return g_pActorPlaceHolderTexture; //default
}

HRESULT __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
    if (!g_pFriskTexture)
    {
        HRESULT hr = D3DXCreateTextureFromFile(pDevice, L"frisk.png", &g_pFriskTexture);
        if (FAILED(hr))
        {
            printf("Failed to load frisk.png texture. HRESULT: 0x%08X\n", hr);
            g_pFriskTexture = nullptr;
        }
        else
        {
            printf("Successfully loaded frisk.png\n");
        }
    }

    if (!g_pActorPlaceHolderTexture)
    {
        HRESULT hr = D3DXCreateTextureFromFile(pDevice, L"Actor_PlaceHolder.png", &g_pActorPlaceHolderTexture);
        if (FAILED(hr))
        {
            printf("Failed to load Actor_PlaceHolder.png texture. HRESULT: 0x%08X\n", hr);
            g_pActorPlaceHolderTexture = nullptr;
        }
        else
        {
            printf("Successfully loaded Actor_PlaceHolder.png\n");
        }
    }

    if (!g_pMainCharaTexture)
    {
        HRESULT hr = D3DXCreateTextureFromFile(pDevice, L"chara_main.png", &g_pMainCharaTexture);
        if (FAILED(hr))
        {
            printf("Failed to load Chara_Main.png texture. HRESULT: 0x%08X\n", hr);
            g_pMainCharaTexture = nullptr;
        }
        else
        {
            printf("Successfully loaded Chara_Main.png\n");
        }
    }

    if (!g_pConsoleTexture)
    {
        HRESULT hr = D3DXCreateTextureFromFile(pDevice, L"console_main.png", &g_pConsoleTexture);
        if (FAILED(hr))
        {
            printf("Failed to load Console_Main.png texture. HRESULT: 0x%08X\n", hr);
            g_pConsoleTexture = nullptr;
        }
        else
        {
            printf("Successfully loaded Console_Main.png\n");
        }
    }

    //font loading (Areal)
    if (!g_pFont)
    {
        InitFont(pDevice);
    }

    if (g_pFriskTexture)
    {
        // Setup render states for 2D sprite rendering
        pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
        pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

        // Draw textured quad with frisk.png
        struct CUSTOMVERTEX
        {
            FLOAT x, y, z, rhw;
            FLOAT tu, tv; // texture coordinates
        };

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_TEX1)

        // Coordinates to draw frisk at position (50,50) with 100x100 size

        if (!gConnectedTCP)
        {
            GameserverHandler::StartMultiplayerThread(GSip, GSport);
            gConnectedTCP = true; //TODO add  connection checks pls
        }

        //here get every player in same room
        CurrentPlayerPosCheckingFrames++;
        if (CurrentPlayerPosCheckingFrames > CurrentPlayerPosNeededFrames)
        {
            if (LocalConnected)
            {
                AllPlayerrvs = GameserverHandler::GetAllRoomIDPlayers(CurrentRoomID, gPlayerGlobalName);
            }
            else
            {
                AllPlayerrvs = GameserverHandler::GetLatestRoomMates();
                AllActorsrvs = GameserverHandler::GetLatestActorRoomMates();
            }

            if (AllPlayerrvs.size() == 0)
            {
                AllPlayerrvs = OldPlayerrvs;
                if (CurrentRoomTicks > TicksNeededToForceUpdate)
                {
                    AllPlayerrvs = GameserverHandler::GetLatestRoomMates();
                    CurrentRoomTicks = 0;
                }
                else
                {
                    CurrentRoomTicks++;
                }
                OldPlayerrvs = AllPlayerrvs;
            }
            else
            {
                OldPlayerrvs = AllPlayerrvs;
            }
            CurrentPlayerPosCheckingFrames = 0;
        }

        for (int i = 0; i < AllPlayerrvs.size(); i++)
        {
            auto Currentrv = AllPlayerrvs.at(i);

            int WindowRealWidth = APlayerController::GetCurrentWindowWidth();
            int WindowRealHeight = APlayerController::GetCurrentWindowHeight();

            if (WindowRealWidth > 641)
            {
                WindowRealWidth = 1280; //because its undertale
                //custom code
                float VirtualWidth = WindowRealWidth;
                float VirtualHeight = WindowRealHeight; // always real height

                float localX = APlayerController::GetPlayerCords(true);
                float remoteX = Currentrv.GetX();
                float distanceX = remoteX - localX;

                float scaleX = VirtualWidth / 320.0f; // e.g. 1280 / 320 = 4.0
                float centerX = (WindowRealWidth / 2.0f) + 300;

                float x = centerX + (distanceX * scaleX);

                // Compensate for black bars

                // Y math (real height based)
                float remoteY = Currentrv.GetY();
                float distanceY = remoteY - gPlayerY;

                float scaleY = WindowRealHeight / 320.0f;
                float centerY = WindowRealHeight / 4.0f;

                float y = centerY + remoteY + 300;

                // Double sprite size
                float width = 48.0f * 2.0f;
                float height = 64.0f * 2.0f;

                if (abs(distanceX) > 575)
                {
                    continue;
                }

                CUSTOMVERTEX quad[] =
                {
                    { x,          y,           0.0f, 1.0f, 0.0f, 0.0f },             // top-left
                    { x + width,  y,           0.0f, 1.0f, 1.0f, 0.0f },             // top-right
                    { x,          y + height,  0.0f, 1.0f, 0.0f, 1.0f },             // bottom-left

                    { x + width,  y,           0.0f, 1.0f, 1.0f, 0.0f },             // top-right
                    { x + width,  y + height,  0.0f, 1.0f, 1.0f, 1.0f },             // bottom-right
                    { x,          y + height,  0.0f, 1.0f, 0.0f, 1.0f }              // bottom-left
                };

                pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
                pDevice->SetTexture(0, g_pFriskTexture);
                pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, quad, sizeof(CUSTOMVERTEX));
            }
            else
            {
                float localX = APlayerController::GetPlayerCords(true);
                float remoteX = Currentrv.GetX();
                float distance = remoteX - localX;

                float scale = 600.0f / 320.0f; // e.g. 3.75
                float center = 600.0f / 2.0f;  // draw local player in center

                float x = center + (distance * scale);

                float remoteY = Currentrv.GetY();
                float distanceY = remoteY - gPlayerY;

                float yscale = 600.0f / 320.0f;  // or whatever scaling is right vertically
                float centerY = 600.0f / 4.0f;

                float y = centerY + Currentrv.GetY(); //pls update for every to get the y oofset
                float width = 48.0f;
                float height = 64.0f;

                CUSTOMVERTEX quad[] =
                {
                    { x,          y,           0.0f, 1.0f, 0.0f, 0.0f },             // top-left
                    { x + width,  y,           0.0f, 1.0f, 1.0f, 0.0f },             // top-right
                    { x,          y + height,  0.0f, 1.0f, 0.0f, 1.0f },             // bottom-left

                    { x + width,  y,           0.0f, 1.0f, 1.0f, 0.0f },             // top-right
                    { x + width,  y + height,  0.0f, 1.0f, 1.0f, 1.0f },             // bottom-right
                    { x,          y + height,  0.0f, 1.0f, 0.0f, 1.0f }              // bottom-left
                };


                pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
                pDevice->SetTexture(0, g_pFriskTexture);
                pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, quad, sizeof(CUSTOMVERTEX));
            }
        }

        //ACTORS


        for (int i = 0; i < AllActorsrvs.size(); i++)
        {
            auto Currentrv = AllActorsrvs.at(i);
            UTexture ActorUTexture(1, 1, 1, true);
            IDirect3DTexture9* CurrentDirectXTexture = nullptr;

            if (i < AllActorIDSprites.size())
            {
                auto spriteID = AllActorIDSprites.at(i);
                ActorUTexture = Textures::GetTexture(spriteID);
                CurrentDirectXTexture = GetDirectXActorTexture(spriteID);
            }

            int WindowRealWidth = APlayerController::GetCurrentWindowWidth();
            int WindowRealHeight = APlayerController::GetCurrentWindowHeight();

            if (WindowRealWidth > 641)
            {
                WindowRealWidth = 1280; //because its undertale
                //custom code
                float VirtualWidth = WindowRealWidth;
                float VirtualHeight = WindowRealHeight; // always real height

                float localX = APlayerController::GetPlayerCords(true);
                float remoteX = Currentrv.GetX();
                float distanceX = remoteX - localX;

                float scaleX = VirtualWidth / 320.0f; // e.g. 1280 / 320 = 4.0
                float centerX = (WindowRealWidth / 2.0f) + 300;

                float x = centerX + (distanceX * scaleX);

                // Compensate for black bars

                // Y math (real height based)
                float remoteY = Currentrv.GetY();
                float distanceY = remoteY - gPlayerY;

                float scaleY = WindowRealHeight / 320.0f;
                float centerY = WindowRealHeight / 4.0f;

                float y = centerY + remoteY + 300;

                // Double sprite size
                float width = ActorUTexture.GetWidth() * 2; //pls update
                float height = ActorUTexture.GetHeight() * 2; //pls update

                if (abs(distanceX) > 575)
                {
                    continue;
                }

                CUSTOMVERTEX quad[] =
                {
                    { x,          y,           0.0f, 1.0f, 0.0f, 0.0f },             // top-left
                    { x + width,  y,           0.0f, 1.0f, 1.0f, 0.0f },             // top-right
                    { x,          y + height,  0.0f, 1.0f, 0.0f, 1.0f },             // bottom-left

                    { x + width,  y,           0.0f, 1.0f, 1.0f, 0.0f },             // top-right
                    { x + width,  y + height,  0.0f, 1.0f, 1.0f, 1.0f },             // bottom-right
                    { x,          y + height,  0.0f, 1.0f, 0.0f, 1.0f }              // bottom-left
                };

                pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
                pDevice->SetTexture(0, CurrentDirectXTexture);
                pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, quad, sizeof(CUSTOMVERTEX));
            }
            else
            {
                UTexture ActorUTexture(1, 1, 1, true);
                IDirect3DTexture9* CurrentDirectXTexture = nullptr;

                if (i < AllActorIDSprites.size())
                {
                    auto spriteID = AllActorIDSprites.at(i);
                    ActorUTexture = Textures::GetTexture(spriteID);
                    CurrentDirectXTexture = GetDirectXActorTexture(spriteID);
                }

                float localX = APlayerController::GetPlayerCords(true);
                float remoteX = Currentrv.GetX();
                float distance = remoteX - localX;

                float scale = 600.0f / 320.0f; // e.g. 3.75
                float center = 600.0f / 2.0f;  // draw local player in center

                float x = center + (distance * scale);

                float remoteY = Currentrv.GetY();
                float distanceY = remoteY - gPlayerY;

                float yscale = 600.0f / 320.0f;  // or whatever scaling is right vertically
                float centerY = 600.0f / 4.0f;

                float y = centerY + Currentrv.GetY(); //pls update for every to get the y oofset
                float width = ActorUTexture.GetWidth();
                float height = ActorUTexture.GetHeight();

                CUSTOMVERTEX quad[] =
                {
                    { x,          y,           0.0f, 1.0f, 0.0f, 0.0f },             // top-left
                    { x + width,  y,           0.0f, 1.0f, 1.0f, 0.0f },             // top-right
                    { x,          y + height,  0.0f, 1.0f, 0.0f, 1.0f },             // bottom-left

                    { x + width,  y,           0.0f, 1.0f, 1.0f, 0.0f },             // top-right
                    { x + width,  y + height,  0.0f, 1.0f, 1.0f, 1.0f },             // bottom-right
                    { x,          y + height,  0.0f, 1.0f, 0.0f, 1.0f }              // bottom-left
                };


                pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
                pDevice->SetTexture(0, CurrentDirectXTexture);
                pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, quad, sizeof(CUSTOMVERTEX));
            }
        }

        //Console

        if (GetAsyncKeyState(VK_OEM_3) & 0x1) { // Key was just pressed
            gConsoleEnabled = !gConsoleEnabled;
        }

        if (gConsoleEnabled)
        {
            std::string ConsoleCommandStr = gCurrentConsoleCommandStr;
            //draw the console duh
            if (GetAsyncKeyState(VK_RETURN) & 0x1) {  // Check if ENTER is pressed
                gConsoleEnabled = false;
                if (!ConsoleCommandStr.empty())
                {
                    std::cout << "Executing Console Command: " << ConsoleCommandStr << "\n";
                    UConsole::ExecuteCommand(ConsoleCommandStr);
                    gCurrentConsoleCommandStr.clear();
                }
            }
            else
            {
                PollConsoleInput();
            }

            //draw the window
            if (APlayerController::GetCurrentWindowWidth() > 641)
            {
                //TODO fullscreen support!
            }
            else
            {
                float screenWidth = 700;
                float barHeight = 25; // Adjust thickness of the bar
                float yTop = 480 - barHeight;
                float yBottom = 480;

                CUSTOMVERTEX quad[] =
                {
                    { 0.0f,          yTop,    0.0f, 1.0f, 0.0f, 0.0f }, // Top-left
                    { screenWidth,   yTop,    0.0f, 1.0f, 1.0f, 0.0f }, // Top-right
                    { 0.0f,          yBottom, 0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-left

                    { screenWidth,   yTop,    0.0f, 1.0f, 1.0f, 0.0f }, // Top-right
                    { screenWidth,   yBottom, 0.0f, 1.0f, 1.0f, 1.0f }, // Bottom-right
                    { 0.0f,          yBottom, 0.0f, 1.0f, 0.0f, 1.0f }  // Bottom-left
                };

                pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
                pDevice->SetTexture(0, g_pConsoleTexture);
                pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, quad, sizeof(CUSTOMVERTEX));

                //double check for font
                if (g_pFont)
                {
                    DrawTextOnScreen("> " + gCurrentConsoleCommandStr, 10, 460, D3DCOLOR_XRGB(255, 255, 255));
                }
            }
        }
    }
    else
    {
        // fallback: draw your old colored rectangle if texture missing
        struct Vertex {
            float x, y, z, rhw;
            DWORD color;
        };

        float x = APlayerController::GetPlayerCords(true);  // base X position
        float y = 50.0f;  // base Y position
        float width = 20.0f;
        float height = 40.0f;

        Vertex body[] = {
            { x,          y,           0, 1, D3DCOLOR_ARGB(255, 150, 75, 0) },          // top-left
            { x + width,  y,           0, 1, D3DCOLOR_ARGB(255, 150, 75, 0) },          // top-right
            { x,          y + height,  0, 1, D3DCOLOR_ARGB(255, 150, 75, 0) },          // bottom-left
            { x + width,  y + height,  0, 1, D3DCOLOR_ARGB(255, 150, 75, 0) }           // bottom-right
        };

        pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
        pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, body, sizeof(Vertex));
    }

    return oEndScene(pDevice);
}

void InitDX9Hook()
{
    if (hookInstalled) return;

    IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D) {
        printf("DX9Hooker: Failed to create Direct3D9 object.\n");
        return;
    }

    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = GetForegroundWindow();

    IDirect3DDevice9* pDevice = nullptr;
    if (FAILED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice))) {
        printf("DX9Hooker: Failed to create dummy D3D9 device.\n");
        pD3D->Release();
        return;
    }

    void** vTable = *reinterpret_cast<void***>(pDevice);
    void* pEndScene = vTable[42];

    if (MH_CreateHook(pEndScene, &hkEndScene, reinterpret_cast<void**>(&oEndScene)) != MH_OK ||
        MH_EnableHook(pEndScene) != MH_OK) {
        printf("DX9Hooker: Failed to hook EndScene.\n");
    }
    else {
        printf("DX9Hooker: Hooked EndScene at %p.\n", pEndScene);
        hookInstalled = true;
    }

    pDevice->Release();
    pD3D->Release();
}

void CleanupDX9Hook()
{
    if (hookInstalled) {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_RemoveHook(MH_ALL_HOOKS);
        hookInstalled = false;
        printf("DX9Hooker: Hooks cleaned up.\n");
    }

    if (g_pFriskTexture) {
        g_pFriskTexture->Release();
        g_pFriskTexture = nullptr;
    }
}
