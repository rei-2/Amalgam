#pragma once
#include "../../../SDK/SDK.h"
#include "../Render.h"
#include <ImGui/TextEditor.h>
#include <mutex>

struct Output_t
{
	std::string m_sFunction;
	std::string m_sLog;
	size_t m_iID;

	Color_t tAccent;
};

class CMenu
{
private:
	void DrawMenu();

	void MenuAimbot(int iTab = 0);
	void MenuHVH(int iTab = 0);
	void MenuVisuals(int iTab = 0);
	void MenuMisc(int iTab = 0);
	void MenuLogs(int iTab = 0);
	void MenuSettings(int iTab = 0);
	void MenuSearch(std::string sSearch);

	void AddDraggable(const char* sLabel, ConfigVar<DragBox_t>& var, bool bShouldDraw = true, ImVec2 vSize = { H::Draw.Scale(100), H::Draw.Scale(40) });
	void AddResizableDraggable(const char* sLabel, ConfigVar<WindowBox_t>& var, bool bShouldDraw = true, ImVec2 vMinSize = { H::Draw.Scale(100), H::Draw.Scale(100) }, ImVec2 vMaxSize = { H::Draw.Scale(1000), H::Draw.Scale(1000) }, ImGuiSizeCallback fCustomCallback = nullptr);
	void DrawBinds();

	std::deque<Output_t> m_vOutput = {};
	size_t m_iMaxOutputSize = 1000;

public:
	void Render();
	void AddOutput(const char* sFunction, const char* sLog, Color_t tColor = Vars::Menu::Theme::Accent.Value);

	bool m_bIsOpen = false;
	bool m_bInKeybind = false;
	bool m_bWindowHovered = false;

	std::mutex m_tMutex;
};

ADD_FEATURE(CMenu, Menu);