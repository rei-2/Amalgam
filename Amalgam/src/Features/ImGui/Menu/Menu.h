#pragma once
#include "../../../SDK/SDK.h"
#include "../Render.h"
#include <ImGui/TextEditor.h>

class CMenu
{
	void DrawMenu();

	void MenuAimbot(int iTab);
	void MenuVisuals(int iTab);
	void MenuMisc(int iTab);
	void MenuLogs(int iTab);
	void MenuSettings(int iTab);

	void AddDraggable(const char* sTitle, ConfigVar<DragBox_t>& info, bool bShouldDraw);
	void DrawBinds();
	void DrawCameraWindow();
	void DrawRadar();

public:
	void Render();

	bool m_bIsOpen = false;
	bool m_bInKeybind = false;
};

ADD_FEATURE(CMenu, Menu);