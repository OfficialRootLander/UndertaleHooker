# Undertale Hooker

#Gameserver coming soon!

## Overview

**Undertale Hooker** lets you play **multiplayer Undertale** and **mod it on runtime** by hooking directly into the game’s DirectX 9 rendering pipeline — without modifying any game source files or executables.

This DLL hooks the `EndScene` function in DirectX 9 to enable custom drawing and multiplayer features in Undertale.

## Console & Commands

### Opening the Console
- Press the **tilde key (~)** to open the console.
- Type a command and press **Enter** to execute it.
- You can also use the **ImGui window** to input commands.

### Commands

| Command                 | What It Does                                                                                  |
|-------------------------|-----------------------------------------------------------------------------------------------|
| `OPEN <IP>`             | Connects to a gameserver at the specified IP address so you can join multiplayer. Example: `OPEN 127.0.0.1` |
| `UNLOCK UTADMIN <CODE>` | Sends a code to the server requesting admin rights. The server must approve it. Example: `UNLOCK UTADMIN 1234` |
| `ROOM <RoomID>`         | Instantly teleports your player to the specified room in the game world. Example: `ROOM 68 (Snowadin` |
| `MOB <MobID>`           | Spawns a character or mob in the current room. Example: `MOB 95` (**ID 95 is Sans**, the iconic Undertale character) |
| `S0`                    | Stops all in-game sound immediately. Useful for muting music or sound effects.               |
| `S1`                    | Enables sound in the game if it was previously stopped or locked.                             |
| `S2`                    | Locks the current audio state so it cannot be changed until unlocked.                        |


## Prerequisites

## Custom Images (Required)

**You MUST place any custom images or textures in your Undertale game folder** — the same folder where `UNDERTALE.exe` is located.  

**Steps:**

1. Find your Undertale installation folder (where `UNDERTALE.exe` is located).  
2. Copy all custom images or textures into this folder.  
3. Launch Undertale with **Undertale Hooker** injected. The DLL will automatically load the images at runtime.  

> ⚠️ This step is required. **Undertale Hooker will not function properly if images are not placed in the game folder.**

### Microsoft DirectX SDK (June 2010) Required

To build and run **Undertale Hooker** successfully, you **must have the Microsoft DirectX SDK (June 2010) installed** on your system.

This SDK provides essential libraries and headers (`d3dx9.lib`, `d3dx9.dll`, `d3dx9.h`) needed for texture loading and hooking DirectX 9.

The DirectX SDK is **not included** with Visual Studio or the default Windows SDK.

You would have to add this!

---

## Download DirectX SDK (June 2010)

Get the official DirectX SDK here:

- [DirectX SDK (June 2010) download](https://www.microsoft.com/en-us/download/details.aspx?id=6812)

---

## Build Instructions

1. Install the DirectX SDK (June 2010).

2. Configure your Visual Studio project to include the SDK directories:

   - **Include directories:**  
     `$(ProgramFiles(x86))\Microsoft DirectX SDK (June 2010)\Include`

   - **Library directories:**  
     `$(ProgramFiles(x86))\Microsoft DirectX SDK (June 2010)\Lib\x86` (or `x64` if building 64-bit)

3. Link the following libraries:  
   `d3dx9.lib`, `d3d9.lib`, and `dxguid.lib`

---

## Runtime Requirements

- The target machine must have the **DirectX End-User Runtimes** installed so that `d3dx9.dll` and related runtime components are available.
- These are usually installed by games or GPU drivers.
- If you encounter missing DLL errors, install the runtimes from:  
  [DirectX End-User Runtime Web Installer](https://www.microsoft.com/en-us/download/details.aspx?id=35)

---

## Important Notes

- This project relies on legacy DirectX SDK components which Microsoft has deprecated.
- For improved portability, consider alternative texture loading methods that do not require the legacy DirectX SDK.

---

## Contact

For questions or support, please contact:  
**[Discord: RootLander]**

---

Thank you for using Undertale Hooker — enjoy modifying Undertale at runtime!
