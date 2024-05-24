#pragma once
#include "../../../SDK/SDK.h"
#include "../Render.h"
#include <ImGui/TextEditor.h>

class CMenu
{
	void DrawMenu();

	void MenuAimbot();
	void MenuVisuals();
	void MenuMisc();
	void MenuLogs();
	void MenuSettings();

	void AddDraggable(const char* szTitle, ConfigVar<DragBox_t>& info, bool bShouldDraw);
	void DrawBinds();
	void DrawCameraWindow();
	void DrawRadar();

	int CurrentTab = 0;

	int CurrentAimbotTab = 0;
	int CurrentVisualsTab = 0;
	int CurrentLogsTab = 0;
	int CurrentConfigTab = 0;
	int CurrentConfigType = 0;

	ImVec2 TabSize = { 65, 72 };
	ImVec2 SubTabSize = { 90, 48 };

	// material editor stuff
	TextEditor TextEditor;
	std::string CurrentMaterial;
	bool LockedMaterial;

public:
	void Render();

	bool IsOpen = false;
	bool ConfigLoaded = false;
	bool InKeybind = false;
};

ADD_FEATURE(CMenu, Menu);