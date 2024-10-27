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

	void AddDraggable(const char* sTitle, ConfigVar<DragBox_t>& info, bool bShouldDraw);
	void DrawBinds();
	void DrawCameraWindow();
	void DrawRadar();

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