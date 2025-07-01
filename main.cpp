#include "menu/includes.h"
#include "menu/imgui_hook.h"
#include "menu/MenuSettings.h"
#include "TextEditor/TextEditor.h"
#include "api.h"

static TextEditor editor;
bool editorInitialized = false;

void ImGuiHook::Render() {
	if (!editorInitialized) {
		editorInitialized = true;
		editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
		editor.SetText("");
	}

	ImGui::SetNextWindowSize(ImVec2(400, 300));
	ImGui::Begin("Aetheris", &settings::enabled, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	
	ImVec2 title_pos = ImGui::GetWindowPos();
	float radius = 5.0f;
	float y_center = title_pos.y + ImGui::CalcTextSize("Aetheris").y * 0.5f + 3.5f;
	float x_offset = ImGui::CalcTextSize("Aetheris").x + 12.f;
	
	ImGui::GetOverlayDrawList()->AddCircleFilled(
		ImVec2(title_pos.x + x_offset, y_center),
		radius,
		settings::injected ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255),
		30
	);

	ImVec2 content_min = ImGui::GetWindowContentRegionMin();
	ImVec2 content_max = ImGui::GetWindowContentRegionMax();
	ImVec2 content_size = ImVec2(content_max.x - content_min.x, content_max.y - content_min.y);
	float btnHeight = ImGui::GetFrameHeight();
	float spacing = ImGui::GetStyle().ItemSpacing.y;
	settings::codePanelSize = ImVec2(content_size.x, content_size.y - btnHeight);
	editor.Render("LuaScriptEditor", settings::codePanelSize);

	bool executePressed = ImGui::Button("Execute");
	if (executePressed) {
		if (!settings::injected) {
			std::string code = editor.GetText();
			Lua::Execute(code.c_str());
		}
		else {
			ImGui::SetTooltip("Join in the game!");
		}
	}

	hook_Run("DrawGui", 0);
	DrawGui();

	ImGui::End();

	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
	ImGui::Begin("Debug");
	ImGui::Text("Rects count: %d", g_drawRects.size());
	ImGui::End();
}

void UpdateStatus() {
	while (true)
	{
		settings::setInjected(Lua::isGameReady());
		Sleep(500);
	}
}

void ImGuiHook::MainThread() {
	CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)UpdateStatus, nullptr, 0, nullptr);
	Lua::Init();
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	SetConsoleTitleA("Aetheris console");
	while (true) {
		Lua::UpdateLuaCoroutines();
		hook_Run("Think", 0);
	}
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		ImGuiHook::Init(hMod);
		break;
	case DLL_PROCESS_DETACH:
		ImGuiHook::Shutdown();
		break;
	}
	return TRUE;
}