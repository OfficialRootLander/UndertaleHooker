#pragma once
#include <d3d9.h>

// Call this once to install the EndScene hook
void InitDX9Hook();

// Optional: call this to clean up (if ever needed)
void CleanupDX9Hook();
