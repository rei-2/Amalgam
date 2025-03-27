#pragma once
#include "../../../SDK/SDK.h"
#include "../Render.h"
#include <ImGui/TextEditor.h>

struct Output_t
{
	std::string m_sFunction;
	std::string m_sLog;
	size_t m_iID;

	Color_t tAccent;
};

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

	std::deque<Output_t> m_vOutput = {};
	size_t m_iMaxOutputSize = 1000;

public:
	void Render();
	void AddOutput(const std::string& sFunction, const std::string& sLog, const Color_t& tColor = Vars::Menu::Theme::Accent.Value);

	bool m_bIsOpen = false;
	bool m_bInKeybind = false;
};

ADD_FEATURE(CMenu, Menu);