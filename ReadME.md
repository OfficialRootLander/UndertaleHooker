# Undertale Hooker

## Overview

**Undertale Hooker** lets you play **multiplayer Undertale** by hooking directly into the game’s DirectX 9 rendering pipeline — without modifying any game source files or executables.

This DLL hooks the `EndScene` function in DirectX 9 to enable custom drawing and multiplayer features in Undertale.

---

## Prerequisites

### Microsoft DirectX SDK (June 2010) Required

To build and run **Undertale Hooker** successfully, you **must have the Microsoft DirectX SDK (June 2010) installed** on your system.

This SDK provides essential libraries and headers (`d3dx9.lib`, `d3dx9.dll`, `d3dx9.h`) needed for texture loading and hooking DirectX 9.

The DirectX SDK is **not included** with Visual Studio or the default Windows SDK.

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
**[Your Name or Contact Info]**

---

Thank you for using **Undertale Hooker** — enjoy multiplayer Undertale without modding the source!
