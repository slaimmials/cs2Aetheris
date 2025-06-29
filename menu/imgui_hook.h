#pragma once
#include <Windows.h>

namespace ImGuiHook {
	extern void Init(HMODULE hMod);
	extern void Shutdown();
	extern void MainThread();
	extern void Render();
}