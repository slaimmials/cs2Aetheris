//MenuSettings.h
#pragma once
#include <Windows.h>
#include "../imgui/imgui.h"

namespace settings {
	extern bool enabled;
	extern bool injected;
	extern ImVec2 windowSize;
	extern ImVec2 codePanelSize;

	extern void setInjected(bool injected);
}